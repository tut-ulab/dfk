#ifndef _MOJI_H
#define _MOJI_H

#include <stdio.h>

/* mojiは、32bitの文字としている。
   string.h と同様な機能を定義する。
   strのかわりに、retsu, charのかわりにmojiを使用する。
*/

/* For Mojiretu Handling, */
/* Mojirets is expressed by short vector terminated by 0. */
typedef unsigned int moji;
/* Length of Mojiretsu */
extern int retsu_len(moji * m);
/* Length Yuusen Hikaku */
extern int retsu_order(moji * x, moji * y);
/* Jisho Hikaku */
extern int retsu_cmp(moji * x, moji * y);
/* Mojiretsu Hikizan, */
extern moji * retsu_subtract(moji * x, moji * y);
/* Mojiretsu Copy Until 0 */
extern moji * retsu_copy(moji *dst, moji *src);
/* Mojiretsu Copy (n) times. */
extern moji * retsu_ncopy(moji *dst, moji* src, int n);
/* Duplicate new mojiretsu */
extern moji * retsu_duplicate(moji *s);
/* Search Positiong of s that contains ss */
extern moji * retsu_search(moji *s, moji *ss);

/* string to mojiretsu conversion */
extern moji * m_from_s(moji * s, char * ss);
/* length as moji */
extern int length_as_m(char *s);
/* mojiretsu to string conversion */
extern char * s_from_m(char * s, moji * ss);
/* length as string */
extern int length_as_s(moji *s); 

/* テキストの中に、含まれている文字数をカウントする。*/
extern int moji_length(char *s);
/* テキストの文字が始まる場所のindexの表を生成する。*/
extern int * moji_index_table(char *str);

extern int hiragana_p(moji c);
extern int katakana_p(moji c);
extern int kanji_p(moji c);
extern int alpha_p(moji c);
extern int punct_p(moji c);

extern int some_moji(moji * m, int (* fn)(moji));
extern int all_moji(moji * m, int (* fn)(moji));


/* 引数に指定したファイルについて、ファイルの終了まで読み、その結果を */
/* retsuとして値として返す。列の領域はヒープに確保する。*/

moji * moji_read_file(FILE * f);
int moji_fputs(moji * m, FILE * f);
int moji_fputc(moji, FILE * f);
  
#endif /* _MOJI_H */


