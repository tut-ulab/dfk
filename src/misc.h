#include <stdio.h>

/* INT: 32 bit integer */
#define INT int

/* ファイルをメモリに読み込む */
extern char * text_read_file(char * file_name);
/* 文字が数字だけからなるかを試験する関数 */
extern int number_syntax(char * c);

void save_suffix(INT * suf, INT size, char *file_name);

int file_exist(char * file);

int file_last_position(char * file);

#ifdef MMAP
char * map_file(char * file_name);
INT file_size(char * file_name);
#endif

