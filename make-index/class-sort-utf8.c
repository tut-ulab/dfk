#include <stdio.h>
#include <stdlib.h>
typedef int (*sortfn)(const void *, const void*);

#define magic 20111115
#define MAX_C 6

static char * text; /* テキストのもとデータ*/

static int    size; /* テキストのバイト数*/

static int    count; /* テキストの文字数 */

static int    id_max; /* ドキュメントの総数 */

struct class_struct
{ 
  int first;
  int last; 
  int length;
  int c[MAX_C];
};

static int  class_count = 0;
static struct class_struct * class_table;

/* suffix structの数は大きいので, この構造体はできるだけ
   小さいほうが望ましい。
   id, commonはその都度計算するという方法もある。
   previous_suffixは ドキュメント数 * 最大Nの配列
   のデータ構造にすることもできる。
   現状の版は, わかりやすさ優先。
*/

struct suffix_struct
{ int position;       /* textのスタートindex */
};

static struct suffix_struct * suffix; /* suffix array */

static FILE * dump_file;

static int  undump(void)\
{ 
  int val;
  fread(&val, sizeof(val), 1, dump_file);
  return val;
}

void read_class_file(void)
{ 
  int i; int j; int val;
  val = undump();
  if(val != magic) {
    fprintf(stderr, "Bad Magic\n");
    exit(1);
  }
  size = undump();
  count = undump();
  class_count = undump();
  id_max = undump();
  val = undump();
  if(val != MAX_C) {
    fprintf(stderr, "BAD MAX_C(%d) for %d", val, MAX_C);
  }
  text = (char *) 
         malloc(sizeof(char)*(size+1));
  if(text == 0) { 
    fprintf(stderr, "text?\n");
    exit(1);
  }

  suffix = (struct suffix_struct *)
    malloc( sizeof(struct suffix_struct) * count);
  if(suffix == 0) {
    fprintf(stderr, "suffix?\n");
    exit(1);
  }
  for(i=0;i<count;i++) {
    val = undump();
    suffix[i].position = val;
  };

  class_table = (struct class_struct *)
    malloc( sizeof(struct class_struct) * class_count);
  if(class_table == 0) {
    fprintf(stderr, "suffix?\n");
    exit(1);
  }
  for(i=0;i<class_count;i++) {
    class_table[i].first = undump();
    class_table[i].last  = undump();
    class_table[i].length = undump();
    for(j=0;j<MAX_C;j++) {
      class_table[i].c[j] = undump();
    }
  }
  fread(text, sizeof(char), size+1, dump_file);
  fclose(dump_file);
}

int class_order(struct class_struct * x, struct class_struct * y)
{
  if(x->first < y->first) return -1;
  if(x->first > y->first) return 1;
  if(x->length < y->length) return -1;
  if(x->length > y->length) return 1;
  return 0;
}

static  FILE * dump_file;

static void dump(int n)
{ 
  fwrite(&n, sizeof(int), 1, dump_file);
}

#define magic_out 20111114
static void dump_suffix_class()
{ 
  int i; int j;
  dump(magic_out);
  dump(size);
  dump(count);
  dump(class_count);
  dump(id_max);
  dump(MAX_C);
  for(i=0;i<count;i++) {
    dump(suffix[i].position);
  };
  for(i=0;i<class_count;i++) {
    dump(class_table[i].first);
    dump(class_table[i].last);
    dump(class_table[i].length);
    for(j=0;j<MAX_C;j++) {
      dump(class_table[i].c[j]);
    }
  }
  fwrite(text, sizeof(char), size+1, dump_file);
  fclose(dump_file);
}


int main(int argc, char ** argv)
{
  if(argc != 1) {
    fprintf(stderr, "Usage %s <input > output\n", argv[0]);
    exit(1);
  }
  dump_file = stdin;
  read_class_file();
  qsort(class_table, class_count, sizeof(struct class_struct), (sortfn)class_order);
  dump_file = stdout;
  dump_suffix_class();
  return 0;
}
