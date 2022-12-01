#include <stdio.h>
#include <stdlib.h>

#define UTF8
#ifdef BYTE
#define CHARACTER_LENGHT(X) (1)
#endif
#ifdef EUC
#define CHARACTER_LENGTH(X) ((((X) & 0x80) == 0)? 1 : 2)
#endif
#ifdef SJIS
#define CHARACTER_LENGTH(X) ((((X) & 0x80) == 0)? 1 : 2)
#endif
#ifdef UTF8
#define CHARACTER_LENGTH(X) ((((X) & 0x80)==0) ? 1 : (((X) & 0xE0) == 0xC0) ? 2 : (((X) & 0xF0) == 0xE0) ? 3 : (((X) & 0xF8) == 0xF0) ? 4 : utf_character_length_error(X))
static int utf_character_length_error(char x)
{
  fprintf(stderr, "UTF encoding error (%ld)\n",(long) x);
  exit(1);
  return 0;
}
#endif


/*
１行を１ドキュメントとして扱い, 
全文字列のcf, df1, df2(その文字列を2回以上含むドキュメントの数), ...
を求める内部データを生成する。その上で,
与えられた文字列のcf, df1, df2, ...を求める。

以下のプログラムは, 文字列が病的でない場合にはN log N (N はファイルの大
きさ)のオーダで前処理ができる。また, 前処理のメモリの使用量はNのオーダである。
cf, df1, df2, df3を求めるのにはlog Nのオーダの計算時間である。


また, コメントにある部分(suffixの生成の部分)を取り替えれば病的な文字列の
場合でも(どんな文字列の場合でも) N log Nにすることができる。この処置は
, 重複の少ないテキストでは必須ではないが, 遺伝子情報の解析などでは必要に
なる。

Nの大きさのメモリのいくつかはドキュメントの数(行数)の大きさの
メモリに取り替える方法がある。

2002/01/11 真田君の変更レポートを参考にして, 日本語化

メモリの負荷を軽減するために, クラスをファイルに
生成する部分だけのプログラムとして、
ソートと２分探索を分離

*/

/* もとめるdf_kのkの最大値+1,
   計算時間, メモリ使用量も この値に比例する。
*/
#define MAX_C 6

/* ドキュメントの区切り */
#define SEPARATOR '\n'

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* qsortの引数に与えるべき関数の型*/
typedef int (*sortfn)(const void *, const void*);

#define MESSAGE_FILE stdout

static char * text; /* テキストのもとデータ*/

static int    size; /* テキストのバイト数*/

static int    count; /* テキストの文字数 */

static int    id_max; /* ドキュメントの総数 */

static int    id; /* 読み込み中のドキュメントの番号 */

static int    common_max; /* 隣り合うsuffixで共通である文字数の最大値 */

struct suffix_struct
{ int position;       /* textのスタートindex */
  int common;         /* 次のsuffixとの共通部分の長さ */
  int id;             /* 対応するドキュメントの番号 */
  int previous_suffix; /* 同一のドキュメントであるsuffixのもっとも近いもの */
};

static struct suffix_struct * suffix; /* suffix array */

static int * last_suffixes; /* previous suffixを作成するための配列 */

static FILE * text_file; /* データのファイル */

struct pending_struct
{ int start_suffix;
  int length;
  int c[MAX_C];
};

static int class_count;

FILE * dump_file;

static void dump(int n)
{ 
  fwrite(&n, sizeof(int), 1, dump_file);
}

#define magic 20111115

static void dump_suffix_class_head(void)
{ int i;
  dump(magic);
  dump(size);
  dump(count);
  dump(class_count);
  dump(id_max);
  dump(MAX_C);
  for(i=0;i<count;i++) {
    dump(suffix[i].position);
  };
}

static void dump_suffix_class_tail(void)
{
  fwrite(text, sizeof(char), size+1, dump_file);
  fclose(dump_file);
}

#ifdef DEBUG
static void debug_print_string(int s, int length)
{ int i; int j; char ch; int len;
  j = 0;
  for(i=0;i<length;i++) { 
    ch = *p;
    len=CHARACTER_LENGTH(ch);
    while(len>0) {
      putchar(*p);
      p++;
      j++;
      len--;
    }
  }
}
#endif

static void dump_one_class(int first, int last, int length, int c[])
{ int i;
#ifdef DEBUG
  printf(" --->      Class[%d, %d] L=%d", first, last, length);
  for(i=0;i<MAX_C;i++) {
    printf(" c[%d]=%d", i, c[i]);
  }
  printf(" S=\"");
  debug_print_string(first, length);
  printf("\"\n");
#endif
  dump(first);
  dump(last);
  dump(length);
  for(i=0;i<MAX_C;i++) {
    dump(c[i]);
  }
}

/* start_suffixとは, いま考慮中のクラスの最初のsuffixでの場所
   lengthとは, いま考慮中のクラスの最大の文字数
   c[0] とは クラスの属する文字列の単独の出現の回数,
   c[1] とは 
     suffixの順番に文字列の出現をならべるという順序をつけておいて、同一ドキュメントに
     自分より小さい番号の出現が２つ現れるという条件でのクラスに属する文字列の出現の数,
   c[2]
     suffixの順番に文字列の出現をならべるという順序をつけておいて、同一ドキュメントに
     自分より小さい番号の出現が３つ現れるという条件でのクラスに属する文字列の出現の数,

重要な性質
 (0) c[0]は通常はtfと呼ばれている数である。
 (1) c[0], c[1], c[2]より, df1, df2がもとまる。
              df_1 は 文字列を１回以上含むドキュメントの数
              df_2 は 文字列を２回以上含むドキュメントの数
   df1 = c[0] - c[1]
   df2 = c[1] - c[2]
   一般に df_k = c_[k-1] - c_[k]
   具体的な図を示すことで証明できるが,
     証明の方針は
       ある文字列が,あるドキュメントに丁度m個出現したとすると,
       そのドキュメントに関するc1, ... c_{m}について、
              c[i] == 1, if (i < m)
              c[i] == 0, if (i >=  m)
       が成立する。
     コーパス全体のc[i]は, 全部のドキュメントに関するc[i]の
     合計をとったものである。

 (2) 包含するクラスの出現数は, 合計でもとまる。
   あるクラスのc[0] = 合計(そのクラスに包含されるクラスのc[0])
   あるクラスのc[1] = 合計(そのクラスに包含されるクラスのc[1])
     一般に
      あるクラスのc[k] = 合計(そのクラスに包含されるクラスのc[k])

   これは, c[k]が, ある条件を満たす文字列の出現だからである。
*/

static struct pending_struct * pendings;
static int level;

/*  外部仕様を明確にする単純な定義
現在の注目点であるsuffix場所から引数で与えられている
suffixの場所の範囲について, それを包含する
pendingになっているクラスを特定する関数
*/

#ifdef DOCUMENTATION
static int pending_level_simple(int suffix)
{ 
  int i;
    for(i = level; i>=0; i--) {
    if(pendings[i].start_suffix <= suffix) return i;
  }
  fprintf(stderr, "pending level not found for %d\n", suffix);
  exit(2);
}
#endif

static int pending_level(int suffix)
{
  int min, max, mid;
  min = 0; max = level; 
  while(min + 1 < max) {
    mid = (min + max) / 2;
    if(pendings[mid].start_suffix <= suffix){
      min = mid;
    } else {
      max = mid;
    }
  }
  if(pendings[max].start_suffix <= suffix) return max;
  if(pendings[min].start_suffix <= suffix) return min;
  fprintf(stderr, "internal error(pending_level)\n");
  exit(2);
  return -1;
}
  

#ifdef DEBUG
/* 指定されたsuffixの状態を表示 */
static void debug_suffix(int i)
{   
  int j;
  fprintf(MESSAGE_FILE,
	    "%5d %3d %5d %2d:", 
	    i, 
	    suffix[i].id, 
	    suffix[i].previous_suffix,
	    suffix[i].common);
  for(j=0;text[suffix[i].position+j]!='\n';j++) {
    fputc(text[suffix[i].position+j], MESSAGE_FILE);
  }
  fputc('\n', MESSAGE_FILE);
  fflush(MESSAGE_FILE);
}

/* suffix arrayの状態を表示 */
static void debug_output()
{ int i;
  fprintf(MESSAGE_FILE, "size = %d\n", size);
  fprintf(MESSAGE_FILE, "id_max = %d\n", id_max);
  fprintf(MESSAGE_FILE, "common_max = %d\n", common_max);
  for(i=0;i<count;i++) {
    debug_suffix(i);
  }
}

/* pending classの状態を表示 */
static void debug_pending()
{ 
  int i; int j;
  for(i = 0; i<=level; i++) {
    fprintf(MESSAGE_FILE, "<S%d L%d",
	    pendings[i].start_suffix,
	    pendings[i].length);
    for(j=0;j<MAX_C;j++) {
      fprintf(MESSAGE_FILE, " %d", pendings[i].c[j]);
    }
    fprintf(MESSAGE_FILE, ">");
  }
  fprintf(MESSAGE_FILE, "\n");
}
#endif

static void error_alloc(void)
{
  fprintf(MESSAGE_FILE, 
	  "text %lx, suffix %lx, last_suffixes %lx, classes %lx \n",
	  (long)text, 
	  (long)suffix, 
	  (long)last_suffixes, 
	  (long)pendings);
  exit(1);
}

/* suffixの順序の決定関数, 
   ドキュメントの区切りで中断するのに注意
   この処理をしないと, strcmpの計算が
   不必要に長くなる可能性があり,
   suffix arrayの計算に異様に時間が
   かかる場合がある。
*/
static int my_strcmp(char *x1, char *y1)
{ 
  register unsigned char * x;
  register unsigned char * y; 
  x = (unsigned char *) x1; y= (unsigned char *)y1;
  while(*x && *y) {
    if(*x < *y) return -1;
    if(*x > *y) return  1;
    if(*x == '\n') break;
    if(*y == '\n') break;
    x++; y++;
  }
  return 0;
}
    
/* suffixが示す文字列の順序を定める関数
   先頭からある部分が同じなら, 次の文字で順序が決定する
   という性質が必要である。
*/
static int suffix_order(struct suffix_struct * x, struct suffix_struct * y)
{
  return my_strcmp(text + x->position, text + y->position);
}

/* 先頭から共通の文字数を求める。
ドキュメントの区切りで中断するのに注意*/
static int common_length(char * x, char * y)
{int i; char xch; char ych; int len;
 i = 0;
 /*while((*x == *y) && (*x) && (*x !='\n')) { i++; x++; y++;}; */
 for(;;){
   xch = *x;
   ych = *y;
   len = CHARACTER_LENGTH(xch);
   while(len>0) {
     if(xch != ych) break;
     if(xch == 0) break;
     if(xch == '\n') break;
     x++; y++;
     xch = *x;
     ych = *y;
     len--;
   }
   if(len>0) break;
   i++;
 }
 return i;
}


/* クラスについて, 場所, 長さ, 文字, 計数値を表示する */
#ifdef DEBUG
static void output_class(int first, int last, int length, int  c[])
{ 
  int i; int j;
  printf("Class[%4d,%4d] L=%d tf=%d",
	 first,
	 last,
	 length,
	 c[0]);
  for(j=1;j<MAX_C;j++) {
    printf(" df%d=%d", j, c[j-1]-c[j]);
  }
  printf(" S=\"");
  debug_print_string(first, length);
  printf("\"\n");
}
#endif



void class_detect(char * file)
{ char * p; int i; int j; int ch; int previous; 
#ifdef DEBUG
   int ii;
#endif
  text_file = fopen(file, "r");
  if(text_file == 0) {
    fprintf(stderr, "File %s not found\n", file);
    exit(1);
  }

  /* データの総文字数とサイズを求める*/ 
  size = 0;
  count = 0;
  while(EOF != (ch = fgetc(text_file))) {
    int len;
    len = CHARACTER_LENGTH(ch);
    size++; 
    count++;
    len--;
    while(len>0){
      ch = fgetc(text_file);
      size ++;
      len --;
    }
  };

  /* fprintf(stderr, "count=%d\n", count); */

  /* 総文字数から, 必要なデータ領域を生成する*/
  text = (char *) malloc (sizeof(char) * (size + 1));
  if(text == 0) error_alloc();
  suffix = (struct suffix_struct *)
    malloc( sizeof(struct suffix_struct) * (count + 1));
  if(suffix == 0) error_alloc();
  fseek(text_file, 0, SEEK_SET);
  p = text;
  id = 0;
  
  /* メモリに読み込むと同時に, ドキュメントの区切りを
     つけていく */
  id_max = 0;
  j = 0;
  for(i=0;i<count;i++) { int len;
    ch = fgetc(text_file); 
    len = CHARACTER_LENGTH(ch);
    text[j] = ch;
    suffix[i].position = j;
    suffix[i].id = id;
    id_max = id + 1;
    j++;
    len--;
    while(len>0) {
      ch = fgetc(text_file);
      text[j] = ch;
      j++;
      len--;
    }
    if(ch == '\n') { id ++; }
  };
  text[size]=0;


  /* Suffixをつくるルーチン*/
  /* 本格的な処理にはデータの繰り返しを考慮したものに取り替える必要がある */
  common_max = 0;
  /* sortの処理が, 計算時間の大半を占めると予想される */
  qsort(suffix, count, sizeof(struct suffix_struct), (sortfn) suffix_order);
  suffix[count].position = size;
  for(i=0;i<count;i++) {
    /* 繰り返しを考慮したルーチンはcommonを同時に求めることができる */
    int c;
    c = common_length(text+suffix[i].position, text+suffix[i+1].position);
    suffix[i].common = c;
    if(c > common_max) common_max = c;
  }
  suffix[count].common = 0;
  /* 取り替えの部分の終了 */


  /* 重複計算のためのデータ構造の生成 */
  last_suffixes = (int *) malloc( id_max * sizeof(int));
  if(last_suffixes == 0) error_alloc();
  for(i=0;i<id_max;i++) { last_suffixes[i] = -1; }
  for(i=0;i<count;i++) {
    suffix[i].previous_suffix = last_suffixes[suffix[i].id];
    last_suffixes[suffix[i].id] = i;
  }

#ifdef DEBUG
  debug_output();
#endif

  /* クラス構造の取出し */

  pendings = (struct pending_struct *) 
    malloc(sizeof(struct pending_struct) * (common_max+1));
  if(pendings == 0) error_alloc();

  /* 1 pass目, class_countを計測する */
  class_count = 0;
  level = 0;
  pendings[level].length = 0;
  pendings[level].start_suffix = 0;
  for(j=0;j<MAX_C;j++) {
    pendings[level].c[j] = 0;
  }
  for(i=0;i<count;i++) {
#ifdef DEBUG
    debug_pending();
    debug_suffix(i);
#endif
    /* 前処理, 新しいクラスの始まりかどうかのチェック */
    if(suffix[i].common > pendings[level].length) {
      /* 現在の場所から新しいクラスを生成する。*/
      /* 新しいクラスは, カウント0から始める */
      level++;
      pendings[level].start_suffix = i;
      pendings[level].length = suffix[i].common;
      for(j=0;j<MAX_C;j++) {	pendings[level].c[j] = 0;      }
    }

    /* 計数処理, 文字列の出現に関して適切なクラスを検索して計数する */
    /* previousを求めるメモリ参照の代りに, MAX_Cが定数であることを
       利用して, MAX_C * max_idの大きさの配列を使うことも可能。
       メモリが厳しいときは, そのように変更した方が良いと思われる。*/
    
    previous = suffix[i].previous_suffix;
    pendings[level].c[0]++;
#ifdef DEBUG
    printf(" -> Incrementing c[%d] of Class*[%d,%d] : (S%d,L%d) S=\"",
	   0,
	   i,
	   i,
	   pendings[level].start_suffix,
	   pendings[level].length
	   );
    debug_print_string(pendings[level].start_suffix,
		       pendings[level].length);

    printf("\"\n");
#endif
    for(j=1;j<MAX_C;j++) {
      int plev;
      if(previous < 0) break;
      plev = pending_level(previous);
#ifdef DOCUMENTATION
      if(plev != pending_level_simple(previous)) {
	fprintf(stderr, "internal error(%d, %d)\n", plev, previous);
	exit(2);
      }
#endif
      pendings[plev].c[j]++;
#ifdef DEBUG
      printf(" -> Incrementing c[%d] of Class*[%d,%d] : (S%d,L%d) S=\"",
	     j,
	     previous,
	     i,
	     pendings[plev].start_suffix,
	     pendings[plev].length
	     );
      debug_print_string(pendings[plev].start_suffix,
			 pendings[plev].length);
      printf("\"\n");
#endif
      previous = suffix[previous].previous_suffix;
    }

    /* 後処理: classの終了の検出 */
    while(suffix[i].common < pendings[level].length) {
      int common = suffix[i].common;
      /* classの終了が発見されたとき */


      /* 最初は, cf>1のクラスだけを数える 
      dump_one_class( 
		     pendings[level].start_suffix,
		     i,                           
		     pendings[level].length,      
		     pendings[level].c);
      */
      class_count++;

      if(level <= 0) { fprintf(stderr, "internal level\n"); exit(2); }
      if( common > pendings[level-1].length) {
	/* 計算中として登録されていなかったクラスが存在した。
	   計算が終了したクラスと同じ場所からスタートする上位の
           クラスの処理を開始 */
	pendings[level].length = common;
	/* 上位のクラスにカウントを引き継ぐ, ただし
           上位のクラスは現在, 計算中のクラスと同じ場所にあるので
           実際の操作は不要である。	*/
#ifdef DOCUMENTATION
        pendings[level].start_suffix = pendings[level].start_suffix;
	for(j=0;j<MAX_C;j++) {
	  pendings[level].c[j] = pendings[level].c[j];
	}
#endif

      }
      if( common <= pendings[level-1].length) {
	/* 上位のクラスのスタート場所は, 今よりも前でpendingになっているもの */
	for(j=0;j<MAX_C;j++) {
	  pendings[level-1].c[j] += pendings[level].c[j];
	}
	level --;
      }
      /* 終了処理をしたあと, 再度終了しているかどうか調べる. */
    }
  }

  dump_file = stdout;
  dump_suffix_class_head(); /* class_countを利用するのに注意 */

  /* 2 pass, 1 passと同じ計算をする。*/

  level = 0;
  pendings[level].length = 0;
  pendings[level].start_suffix = 0;
  for(j=0;j<MAX_C;j++) {
    pendings[level].c[j] = 0;
  }
  for(i=0;i<count;i++) {
#ifdef DEBUG
    debug_pending();
    debug_suffix(i);
#endif
    /* 前処理, 新しいクラスの始まりかどうかのチェック */
    if(suffix[i].common > pendings[level].length) {
      /* 現在の場所から新しいクラスを生成する。*/
      /* 新しいクラスは, カウント0から始める */
      level++;
      pendings[level].start_suffix = i;
      pendings[level].length = suffix[i].common;
      for(j=0;j<MAX_C;j++) {	pendings[level].c[j] = 0;      }
    }

    /* 計数処理, 文字列の出現に関して適切なクラスを検索して計数する */
    /* previousを求めるメモリ参照の代りに, MAX_Cが定数であることを
       利用して, MAX_C * max_idの大きさの配列を使うことも可能。
       メモリが厳しいときは, そのように変更した方が良いと思われる。*/
    
    previous = suffix[i].previous_suffix;
    pendings[level].c[0]++;
#ifdef DEBUG
    printf(" -> Incrementing c[%d] of Class*[%d,%d] : (S%d,L%d) S=\"",
	   0,
	   i,
	   i,
	   pendings[level].start_suffix,
	   pendings[level].length
	   );
    debug_print_string(pendings[level].start_suffix,
		       pendings[level].length);

    printf("\"\n");
#endif
    for(j=1;j<MAX_C;j++) {
      int plev;
      if(previous < 0) break;
      plev = pending_level(previous);
#ifdef DOCUMENTATION
      if(plev != pending_level_simple(previous)) {
	fprintf(stderr, "internal error(%d, %d)\n", plev, previous);
	exit(2);
      }
#endif
      pendings[plev].c[j]++;
#ifdef DEBUG
      printf(" -> Incrementing c[%d] of Class*[%d,%d] : (S%d,L%d) S=\"",
	     j,
	     previous,
	     i,
	     pendings[plev].start_suffix,
	     pendings[plev].length
	     );
      debug_print_string(pendings[plev].start_suffix,
			 pendings[plev].length);
      printf("\"\n");
#endif
      previous = suffix[previous].previous_suffix;
    }

    /* 後処理: classの終了の検出 */
    while(suffix[i].common < pendings[level].length) {
      int common = suffix[i].common;
      /* classの終了が発見されたとき */
      dump_one_class( 
		     pendings[level].start_suffix,/*start suffix */
		     i,                           /*final suffix */
		     pendings[level].length,      /*maximum class length */
		     pendings[level].c);
      if(level <= 0) { fprintf(stderr, "internal level\n"); exit(2); }
      if( common > pendings[level-1].length) {
	/* 計算中として登録されていなかったクラスが存在した。
	   計算が終了したクラスと同じ場所からスタートする上位の
           クラスの処理を開始 */
	pendings[level].length = common;
	/* 上位のクラスにカウントを引き継ぐ, ただし
           上位のクラスは現在, 計算中のクラスと同じ場所にあるので
           実際の操作は不要である。	*/
#ifdef DOCUMENTATION
        pendings[level].start_suffix = pendings[level].start_suffix;
	for(j=0;j<MAX_C;j++) {
	  pendings[level].c[j] = pendings[level].c[j];
	}
#endif

      }
      if( common <= pendings[level-1].length) {
	/* 上位のクラスのスタート場所は, 今よりも前でpendingになっているもの */
	for(j=0;j<MAX_C;j++) {
	  pendings[level-1].c[j] += pendings[level].c[j];
	}
	level --;
      }
      /* 終了処理をしたあと, 再度終了しているかどうか調べる. */
    }
  }
  /* fprintf(stderr, "class=%d\n", class_count); */

#ifdef DEBUG
  debug_pending();
#endif
  dump_suffix_class_tail();
}

int main(int argc, char ** argv)
{
  if(argc != 2) {
    fprintf(stderr, "Usage %s filename > output\n", argv[0]);
    exit(1);
  }
  class_detect(argv[1]);
  exit(0);
  return 0;
}
