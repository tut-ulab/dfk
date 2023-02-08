/* Fixed Bug for 64 bit pointer on reading class file */
/* extended so that it can support multiple class files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "moji_utf8.h"
#include "misc.h"
#include "df.h"

#ifdef DF_MAIN
static FILE * query_file;
static int  query_count;
static char query_line[1000000];
// static moji query_retsu[1000000];
static char query_id[128];
#endif

#define magic 20111114
#define MAX_C 6

static char * text; /* テキストのもとデータ*/

static int    size; /* テキストのバイト数*/

static int    count; /* テキストの文字数 */

static int    id_max; /* ドキュメントの総数 */


int df_total_document() {
  return id_max;
}

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

static int string_sub(char *s1, char *s2)
{ 
  while(*s2) {
    if(*s1 != *s2) { return 0; }
    s1++;
    s2++;
  }
  return *s1;
}

#ifdef MMAP
void * df_setup(const char * file)
{ 
  int * p;
  p = (int *)map_file(file);
  if(p == 0) { fprintf(stderr, "file(%s) not found\n", file); exit(1); }
  df_use(p);
  return (void *) p;
}
#else
static FILE * dump_file;

static int  undump(void)\
{ 
  int val;
  fread(&val, sizeof(val), 1, dump_file);
  return val;
}

void * df_setup(const char * file)
{ void * r;
  int r_size;
  int * head;
  int i; int j; int val;
  dump_file = fopen(file, "r");
  if(dump_file == 0) { fprintf(stderr, "file (%s) not found\n", file); goto error; }
  val = undump();
  if(val != magic) {
    fprintf(stderr, "Bad Magic\n");
    goto error;
  }
  size = undump();
  count = undump();
  class_count = undump();
  id_max = undump();
  val = undump();
  r_size = sizeof(int) * 6; /* header size*/
  r_size += sizeof(struct suffix_struct) * count; /* suffix size */
  r_size += sizeof(struct class_struct) * class_count; /* class size */
  r_size += sizeof(char *) * (size + 1); /* text size */
  r = (void *) malloc(r_size);
  if(r == 0) { fprintf(stderr, "malloc fail\n"); exit(1); }
  head = (int *) r;
  head[0] = magic; head[1] = size; head[2] = count; 
  head[3] = class_count; head[4] = id_max ; head[5] = val;
  

  if(val != MAX_C) {
    fprintf(stderr, "BAD MAX_C(%d) for %d", val, MAX_C);
  }
  //  fprintf(stderr, "size=%d count=%d class=%d\n",  size, count, class_count);
  /*
    suffix = (struct suffix_struct *)
    malloc( sizeof(struct suffix_struct) * count);
    if(suffix == 0) {
    fprintf(stderr, "suffix?\n");
    exit(1);
    }*/
  
  suffix = (struct suffix_struct *) 
    (r + 
     sizeof(int) * 6);

  for(i=0;i<count;i++) {
    val = undump();
    suffix[i].position = val;
  };

  /*
  class_table = (struct class_struct *)
    malloc( sizeof(struct class_struct) * class_count);
  if(class_table == 0) {
    fprintf(stderr, "suffix?\n");
    exit(1);
  }
  */

  class_table = (struct class_struct *) 
    (r + 
     sizeof(int) * 6 + 
     sizeof(struct suffix_struct) * count);

  for(i=0;i<class_count;i++) {
    class_table[i].first = undump();
    class_table[i].last  = undump();
    class_table[i].length = undump();
    for(j=0;j<MAX_C;j++) {
      class_table[i].c[j] = undump();
    }
  }

  /*
  text = (char *) 
         malloc(sizeof(char)*(size+1));
  if(text == 0) { 
    fprintf(stderr, "text?\n");
    exit(1);
  }
  */

  text = (char *)
        (r + 
	 sizeof(int) * 6 + 
	 sizeof(struct suffix_struct) * count +
         sizeof(struct class_struct) * class_count);
  fread(text, sizeof(char), size+1, dump_file);
  fclose(dump_file);

  return r;

 error:
  return (void *) 0;
}



#endif

void df_use(void * table)
{  
  int * p;
  static void * last_table = 0;
  if(table == last_table) return;
  last_table = table;
  p = (int *) table;
  if(p == 0) { fprintf(stderr, "df_use using null pointer"); exit(1); }
  if(p[0] != magic) {
    fprintf(stderr, "Bad Magic\n");
    exit(1);
  }
  size = p[1];
  count = p[2];
  class_count = p[3];
  id_max = p[4];
  if(p[5] != MAX_C) {
    fprintf(stderr, "BAD MAX_C(%d) for %d", p[5], MAX_C);
  }

  suffix = (struct suffix_struct *) &p[6];
  class_table = (struct class_struct *) (((long) suffix) + sizeof(struct suffix_struct) * count);
  text = (char *)( ((long) class_table) + sizeof(struct class_struct) * class_count );
  /*
  fprintf(stderr, "%x(%d) %x(%d) %x(%d)\n", 
	          (int) suffix, sizeof(struct suffix_struct), 
	          (int) class_table, sizeof(struct class_struct),
                  (int) text, sizeof(char));
  fflush(stderr);
  */
}


static int df_class_string_length(char *s)
{ return length_as_m(s); }

/*
static int df_class_string_length(char *s)
{ int i; char ch;
  i = 0;
  for(;;) {
    ch = *s++;
    i++;
    if(ch == 0) break;
    if(ch & 0x80) {
      ch = *s++;
      if(ch == 0) break;
    }
  }
  return i;
  }*/


static int df_class_compare_string(char *x, char *s)
{
  if(string_sub(x, s) != 0) return 0;
  return( my_strcmp(x, s) );
}

static int df_class_compare(int m, char *s)
{ 
  char *x; int r;
  x = text + suffix[class_table[m].first].position;
  r = df_class_compare_string(x, s);
  if(r != 0) return r;
  if (class_table[m].length > df_class_string_length(s)) return 1;
  if (class_table[m].length < df_class_string_length(s)) return -1;
  return 0;
}
  
  

static int df_class_binary(char * s)
{ 
  int min; int max; int mid; int cmp;
  /* cf>=2のclass にあるかどうか検索する */
  min = 0;
  max = class_count-1;
  while(min+1<max) {
    mid = (max + min) / 2;
    cmp = df_class_compare(mid, s);
    if(cmp <  0) { min = mid; } else {max =mid; }
  };
  if((string_sub(text+suffix[class_table[min].first].position, s) != 0) &&
     (string_sub(text+suffix[class_table[min].last].position, s) != 0)
     ) return min;
  if((string_sub(text+suffix[class_table[max].first].position, s) != 0) &&
     (string_sub(text+suffix[class_table[max].last].position, s) != 0)
     ) return max;
  /* cf=1であるかどうかどうか検索する */
  min = 0;
  max = count-1;
  while(min+1<max) {
    mid = (max + min) / 2;
    cmp = df_class_compare_string(text+suffix[mid].position, s);
    if(cmp <  0) {  min = mid; } else {  max = mid; }
  }
  if(string_sub(text+suffix[min].position, s) != 0) return -1;
  if(string_sub(text+suffix[max].position, s) != 0) return -1;
  return -2;
}

static int df_class(char * s)
{ 
  int c;
  c = df_class_binary(s);
#ifdef DOCUMENTATION
  if(c != df_class_simple(s)) {
    fprintf(stderr, "%d %d %s\n", c, df_class_simple(s), s);
  }
#endif
  return c;
}


int cf(const char *s)
{ 
  int c;
  c = df_class(s);
  if(c < -1) return 0;
  if(c < 0) return 1;
  return class_table[c].c[0];
}


static int df1(char *s)
{ 
  int c;
  c = df_class(s);
  if(c < -1) return 0;
  if(c < 0) return 1;
  return class_table[c].c[0] - class_table[c].c[1];
}

int dfn(int k, const char *s)
{ int c;
  if(k>= MAX_C) {
    fprintf(stderr, "%d: dfn K too large\n", k);
  }
  if(k==1) return df1(s);
  c = df_class(s);
  if(c< 0) return 0;
  return class_table[c].c[k-1] - class_table[c].c[k];
}

#ifdef DF_MAIN
	    
char line[1024];
	    

void print_statistics(char * buffer)
{ // int start;
  // int sublength;
  // int length;
  int tf;
  int df;
  int df2;
  int df3;
  int df4;
  int df5;
  int total;

  total = id_max;

  tf = cf(buffer);
  df = dfn(1, buffer);
  df2 = dfn(2, buffer);
  df3 = dfn(3, buffer);
  df4 = dfn(4, buffer);
  df5 = dfn(5, buffer);
  printf("%7d %7d %7d %7d %7d %7d %7d %s %s\n",
	 tf, total, df, df2, df3, df4, df5,
	 query_id, buffer);
  fflush(stdout);
}

int main(int argc, char ** argv)
{ 
  if(argc != 2) {
    fprintf(stderr, "Usage %s filename > output\n", argv[0]);
    exit(1);
  }
  df_setup(argv[1]);
#ifdef DEBUG
  fprintf(stdout, "total=%d\n", class_count);
  for(i=0;i<class_count;i++) {
    output_class( class_table[i].first,
		  class_table[i].last,
		  class_table[i].length,
		  class_table[i].c
		 );
  }		 
#endif
  query_file = stdin;
  if(query_file == 0) { goto error; }
  query_count = 0;
  while(fgets(query_line, sizeof(query_line), query_file)) {
    int n;
    query_count++;
    n = strlen(query_line);
    if(query_line[n-1] == '\n') { query_line[n-1] = 0; } else { goto syntax; };
    sprintf(query_id, "%d", query_count);
    print_statistics(query_line);
    // m_from_s(query_retsu, query_line);
    // print_retsu_statistics(query_retsu);
  }
  exit(0);
 error:
  fprintf(stderr, "Usage: %s data_file < in > out\n", argv[0]);
  exit(1);
syntax:
  fprintf(stderr, "Data Format Error %s \n", query_line);
  exit(1);
  return 1;
}

#endif
