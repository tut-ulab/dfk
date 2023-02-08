#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "misc.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif



char * text_read_file(char * file_name)
{
  FILE * f;
  INT size = 16;
  char * buffer;
  char * new_buffer;
  INT point = 0;
  signed char ch;
  int finish = 0;

  f = fopen(file_name, "r");
  if(f == 0) return(0);

  buffer = 0;
  do {
    if(EOF == (ch = fgetc(f))) {
      ch = 0;
      finish = 1; /*true */
    };
    /* debug putchar(ch); */
    if((buffer == 0) || (point >= size)) {
      new_buffer = (char *) malloc( 2 * size * sizeof(char));
      if(new_buffer == 0) {
	fprintf(stderr, "malloc fail in read_file\n");
	exit(1);
      };
      if(buffer != 0) { 
	memcpy(new_buffer, buffer, size * sizeof(char)); 
	free(buffer);
      };
      buffer = new_buffer;
      size = size + size;
      /* fprintf(stderr, "expand = %d\n", size); */
    };
    buffer[point] = ch;
    point++;
  } while( finish == 0 );
  fclose(f);
  return(buffer);
}

/* Moved to "moji_utf8.c" because it depends the coding of moji"
// テキストの中に、含まれている文字数をカウントする。
INT text_length(char *s)
{ INT r;
  char ch;
  r = 0;
  while((ch = *s++)){
    if(ch & 0x80) {
       ch = *s++;
     };
    r++;
  };
  return(r);
}

// テキストの文字が始まる場所のindexの表を生成する。
INT * text_index_table(char *str)
{ INT n, i;
  INT * r;
  char * s;
  char ch;
  s = str;
  n = 0;
  while((ch = *s++)){
    if(ch & 0x80) {
       ch = *s++;
     };
    n++;
  };
  r = (INT *) malloc( n * sizeof(INT));
  if(r == 0) { fprintf(stderr, "text_table(malloc)\n"); exit(1); };
  s = str;
  i = 0;
  while((ch = *s)) {
    r[i] = s - str;
    s++;
    if(ch & 0x80) {
      ch = *s++;
    };
    i++;
  };
  if(i != n) { fprintf(stderr, "text_table(i)\n"); exit(1); };
  return(r);
}
*/

/* number syntax */
int number_syntax(char *s)
{ char ch;
  while((ch = *s++)) {
    if(ch < '0') return 0;
    if(ch > '9') return 0;
  };
  return 1;
}
    
void save_suffix(INT * suf, INT size, char *file_name)
{   int k;
    FILE * f;
    char out[4];
    int p;
    f = fopen(file_name, "w");
    if(f == 0) goto skip;
    for(k=0;k<size;k++) {
      p = suf[k];
      if((p & 0xffffffff) != p) {
	fprintf(stderr, "original file to to big for this suffix format\n");
	exit(1);
      }
      out[0] = 0xff & (p >> 24);
      out[1] = 0xff & (p >> 16);
      out[2] = 0xff & (p >> 8);
      out[3] = 0xff & p;
      fwrite(out, 1, sizeof(out), f);
    }
  skip:
    fclose(f);
}

/* Removed since, it word_order dependent
INT * load_suffix(INT size, char *file_name)
{ FILE * f;
  INT * r;
  f = fopen(file_name, "r");
  if(f == 0) goto error;
  r = (INT * )malloc(size * sizeof(INT));
  if(r == 0) goto error;
  fread(r, size, sizeof(INT), f);
 error:
  fclose(f);
  return r;
}
*/

int file_exist(char * file_name)
{ FILE * f;
  f = fopen(file_name, "r");
  if(f == 0) { return(0); };
  fclose(f);
  return(1);
}

int file_last_position(char * file_name)
{ FILE * f;
  int p;
  f = fopen(file_name, "r");
  if(f == 0) { return(0); }
  fseek(f, 0L, SEEK_END);
  p = ftell(f);
  fclose(f);
  return(p);
}



#ifdef MMAP
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

    
char * map_file(char * file_name)
{
    struct stat stat_buf;
    void * text;
    int n;
    int fd = open(file_name, O_RDONLY);
    if(fd == -1) goto error;
    if(fstat(fd, &stat_buf) == -1) goto error;
    n = stat_buf.st_size;
#ifdef mips4    
    text = (char *)mmap64(NULL, n+1, PROT_READ /* | PROT_WRITE*/, 
			MAP_PRIVATE, fd, 0);
#else
    text = (char *)mmap(NULL, n+1, PROT_READ /* | PROT_WRITE*/, 
			MAP_PRIVATE, fd, 0);
#endif
    if(text == MAP_FAILED) goto error;

    /*    text[n] = 0;	 pad with null */
    
    return(text);
  error:
    fprintf(stderr, "mmap(%s) failed.\n", file_name);
    return(text_read_file(file_name));
}

INT file_size(char * file_name)
{
    struct stat stat_buf;
    INT n;
    int fd = open(file_name, O_RDONLY);
    if(fd == -1) goto error;
    if(fstat(fd, &stat_buf) == -1) goto error;
    n = stat_buf.st_size;
    close(fd);
    return(n);
  error:
    return(0);
}
#endif

