#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "moji_utf8.h"

#define CHARACTER_LENGTH(X) ((((X) & 0x80)==0) ? 1 : (((X) & 0xE0) == 0xC0) ? 2 : (((X) & 0xF0) == 0xE0) ? 3 : (((X) & 0xF8) == 0xF0) ? 4 : utf_character_length_error(X))
static int utf_character_length_error(char x)
{
  fprintf(stderr, "UTF encoding error (%ld)\n",(long) x);
  exit(1);
  return 0;
}

typedef moji * retsu;

moji *  m_from_s(moji * dst, char *s)
{ unsigned char ch; unsigned char ch1; moji m; moji * d;
  d = dst;
  while((ch = *s++)) {
    int len = CHARACTER_LENGTH(ch);
    m = ch;
    while(len>1) { ch1 = *s++; m = (m << 8) + ch1; len--; }
    *d++ = m;
  };
  *d = 0;
  return(d);
}

int length_as_m(char * s)
{ unsigned char ch; unsigned char ch1; moji m; int r;
  r = 0;
  while((ch = *s++)) {
    int len = CHARACTER_LENGTH(ch);
    m = ch;
    while(len>1) { ch1 = *s++; m = (m << 8) + ch1; len--; }
    r++;
  };
  return(r);
}


char * s_from_m(char * dst, moji * r)
{ register moji m; char * d;
  d = dst;
  while((m = *r++)) {
    if((m & 0xff000000) != 0 ) { *d++ = (m & 0xff000000) >> 24; }
    if((m & 0xff0000) != 0 ) { *d++ = (m & 0xff0000) >> 16; }
    if((m & 0xff00) != 0) { *d++ = (m & 0xff00) >> 8; }
    if((m & 0xff) != 0) { *d++ = (m & 0xff); }
  }
  *d = 0;
  return dst;
}

int length_as_s( moji * r)
{ register moji m; int c;
  c = 0;
  while((m = *r++)) {
    if((m & 0xff000000) != 0 ) { c++; }
    if((m & 0xff0000) != 0 )  { c++; }
    if((m & 0xff00) != 0)  { c++; }
    if((m & 0xff) != 0)  { c++; }
  }
  return c;
}
  

int retsu_len(moji * m)
{ int i;
  i = 0;
  while(*m++) i++;
  return(i);
}

int retsu_order(moji * x, moji * y)
{ int lx; int ly;
  lx = retsu_len(x);
  ly = retsu_len(y);
  if (lx == ly) { return retsu_cmp(x, y);  }
  else { return (lx > ly) ? 1 : -1; };
}

int retsu_cmp(moji * x, moji *y)
{ moji lx; moji ly;
  for(;;) {
    lx = *x++; ly = *y++;
    if(lx == 0) {
      if(ly == 0) { return 0;} else { return -1; };
    };
    if(ly == 0)  return(1);
    if(lx > ly)  return(1);
    if(lx < ly)  return(-1);
  };
}
      
  

moji * retsu_subtract(moji * x, moji * y)
{ register int kx, ky;
  for(;;) {
    ky = *y++;
    if(ky == 0) { return x; };
    kx = *x++;
    if(kx == 0) { return 0; };
    if(kx != ky) { return 0; };
  };
}

retsu retsu_copy(moji * dst, moji *s)
{ moji * d;
  d = dst;
  while((*d++ = *s++));
  return(dst);
}

retsu retsu_ncopy(moji * dst, moji *s, int n)
{ moji * d;
  d = dst;
  while(n>0) { *d++ = *s++; n--; };
  return(dst);
}

moji* retsu_duplicate(moji * s)
{ moji * r;
  r = (moji *) malloc((retsu_len(s)+1)*sizeof(moji));
  if(r) { retsu_copy(r, s); };
  return (r);
}
  
moji * retsu_search(moji * s, moji * sub)
{ for(;;) {
    if(*s == 0) { return 0; }
    if(retsu_subtract(s, sub)) return( s);
    s++;
  };
}


#define T 1

moji * moji_read_file(FILE * f)
{
  long size = 16;
  moji * buffer;
  moji * new_buffer;
  moji mch;
  long point = 0;
  signed char ch, ch1;
  unsigned char uch, uch1;
  int finish = 0;
  int len;

  buffer = 0;
  do {
    if(EOF == (ch = fgetc(f))) {
      ch = 0;
      finish = T;
/*    } else { fputc(ch, stderr); */
    };
    if((buffer == 0) || (point >= size)) {
      new_buffer = (moji *) malloc( 2 * size * sizeof(moji));
      if(new_buffer == 0) {
	fprintf(stderr, "malloc fail in mojiio\n");
	exit(1);
      };
      if(buffer != 0) { 
	memcpy(new_buffer, buffer, size * sizeof(moji)); 
	free(buffer);
      };
      buffer = new_buffer;
      size = size + size;
      /* fprintf(stderr, "expand = %d\n", size); */
    };
    len = CHARACTER_LENGTH(ch);
    uch=ch;
    mch=uch;
    while(len>1) {
      ch1 = fgetc(f);
      uch1 = ch1;
      mch = (mch << 8) + uch1;
      len--;
    }
    buffer[point] = mch;
    point++;
  } while( finish == 0 );
  return(buffer);
}

int moji_length(char *s)
{ return length_as_m(s); }


// テキストの文字が始まる場所のindexの表を生成する。
int * moji_index_table(char *str)
{ int n, i;
  int * r;
  char * s;
  char ch;
  s = str;
  n = moji_length(str);
  r = (int *) malloc( n * sizeof(int));
  if(r == 0) { fprintf(stderr, "moji_index_table(malloc)\n"); exit(1); };
  s = str;
  i = 0;
  while((ch = *s)) {
    int len;
    len = CHARACTER_LENGTH(ch);
    r[i] = s - str;
    s++;
    while(len>1) {
      ch = *s++;
      len--;
    };
    i++;
  };
  if(i != n) { fprintf(stderr, "moji_index_table(i)\n"); exit(1); };
  return(r);
}


int moji_fputs(moji * r, FILE * f)
{ moji m;
  while((m = *r++)) {
    if(m  & 0xff000000) { fputc( (m & 0xff000000) >> 24, f); }
    if(m  & 0xff0000) { fputc( (m & 0xff000000) >> 16, f); }
    if(m  & 0xff00) { fputc( (m & 0xff000000) >> 8, f); }
    if(m  & 0xff) { fputc( (m & 0xff000000) , f); }
  };
  return(0);
}

int moji_fputc(moji m, FILE * f)
{ 
    if(m  & 0xff000000) { fputc( (m & 0xff000000) >> 24, f); }
    if(m  & 0xff0000) { fputc( (m & 0xff000000) >> 16, f); }
    if(m  & 0xff00) { fputc( (m & 0xff000000) >> 8, f); }
    if(m  & 0xff) { fputc( (m & 0xff000000) , f); }
    return(0);
}


static char * alpha_string ="ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static moji * alpha_moji = 0;

static char * hira_string = "あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもがぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽぁぃぅぇぉゃゅょやゆよわらりるれろをん";
static moji * hira_moji = 0;

static char * kata_string = "アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポァィゥェォャュョヤユヨワラリルレロヲン";
static moji * kata_moji = 0;

static char * symbol_string = "。、!@#$%^&*())-_=+{}[]:;\"\'\\|<>?,./ ！＠＃＄％＾＆＊（））−＿＝＋｛｝［］：；”’¥｜＜＞？，．／　・";
static moji * symbol_moji = 0;

static int moji_set_ok = 0;

static moji *  as_moji(char *s)
{ moji * r;
  r = (moji *) malloc(sizeof(moji)*length_as_m(s) + 1);
  if(r == 0) { fprintf(stderr, "as_moji\n"); exit(1); }
  m_from_s(r, s);
  return r;
}

static void init_moji_set(void)
{  
  alpha_moji = as_moji(alpha_string);
  hira_moji  = as_moji(hira_string);
  kata_moji  = as_moji(kata_string);
  symbol_moji = as_moji(symbol_string);
  moji_set_ok = 1;
  return;
}

static int moji_search(moji * s, moji c) {
  moji c1;
  while((c1 = *s++)) {
    if(c1 == c) return (c1 == c);
  }
  return 0;
}

extern int hiragana_p(moji c)
{ if(moji_set_ok == 0) init_moji_set();
  return( moji_search(hira_moji, c) );
}

extern int katakana_p(moji c)
{ if(moji_set_ok == 0) init_moji_set();
  return( moji_search(kata_moji, c));
}

extern int alpha_p(moji c)
{ if(moji_set_ok == 0) init_moji_set();
  return(moji_search(alpha_moji,c)); 
}

extern int punct_p(moji c)
{ if(moji_set_ok == 0) init_moji_set();
  return(moji_search(symbol_moji, c));
}

extern int kanji_p(moji c)
{ if(moji_set_ok == 0) init_moji_set();
  return ( ( hiragana_p(c) == 0) &&
	   ( katakana_p(c) == 0) &&
	   ( alpha_p(c) == 0) &&
	   ( punct_p(c) == 0) ) ;
}

extern int some_moji(moji * m, int (* fn)(moji))
{ moji * p; int status;
  p = m;
  while(*p) {
    status = (*fn)(*p);
    if(status) return status;
    p++;
  }
  return (0);
}

extern int all_moji(moji * m, int (* fn)(moji))
{ moji * p; int status;
  p = m;
  while(*p) {
    status = (*fn)(*p);
    if(status == 0) return status;
    p++;
  }
  return status;
}



#ifdef DEBUG_MAIN

char line_string[10240];
moji line[10240];

void main()
{
  moji * p;
  int status;
  while(fgets(line_string, sizeof(line_string), stdin)) {
    if(strlen(line_string)>=sizeof(line_string)) {
      line_string[64] = 0;
      fprintf(stderr, "long line(%s)\n", line_string);
      exit(1);
    }
    m_from_s(line, line_string);
    p = line;
    status = 0;
    while(*p) {
      if(hiragana_p(*p)) {
	status = 1;
	break;
      }
      p++;
    }
    if(status == 0) {
      fputs(line_string, stdout);
    }
  }
  exit(0);
}
    

#endif





  

  
  


