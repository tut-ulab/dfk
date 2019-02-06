#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "df.h"

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

#define LLEN 1000000

char line_buffer [LLEN];
int  positions [LLEN];

int main(int argc, char **argv){
  void * select_space;
  void * domain_space;
  if(argc != 3) { 
    fprintf(stderr, "Usage %s selected_space.class domain_space.class < Text.dat\n", argv[0]);
    exit(1);
  }

  select_space = df_setup(argv[1]);
  domain_space = df_setup(argv[2]);
  
  while(fgets(line_buffer, sizeof(line_buffer), stdin) ) {
    int n; char *id; char * abst;
    char * p;
    int i;

    /* float b = pow(2.0, 31.0) - 1.0; */

    p = line_buffer;
    
    id = p;
    while(*p && (*p !='\t') && (*p != '\n')) p++;
    if(*p) { *p = 0; p++; }

    abst = p;
    while(*p && (*p !='\t') && (*p != '\n')) p++;
    if(*p) { *p = 0; p++; }

    if(*p) { fprintf(stderr, "too many field : %s\n", id); exit(1); }
    
    p = abst;
    n = 0;

    while(*p) {
      int c = *p;
      int k = CHARACTER_LENGTH(c);
      positions[n] = p - abst; n++;
      p += k;
    }
    positions[n] = p - abst; n++;

    for(i=0;i<n-2;i++) {
      int df_select;
      int df_domain;
      int save;
      float r;
      save = abst[positions[i+2]];
      abst[positions[i+2]] = 0;
      df_use(select_space);df_select=dfn(1, &abst[positions[i]]);
      df_use(domain_space);df_domain=dfn(1, &abst[positions[i]]);
      r = (float) df_select / ((float) df_domain + 0.5);
      printf("%s\t%d\t%d\t%s\t%10.5f\t%s\n", id, i, 2, &abst[positions[i]], r, "-");
      abst[positions[i+2]] = save;
    }
  }
}

