#include <stdio.h>

/* INT: 32 bit integer */
#define INT int

/* �ե�����������ɤ߹��� */
extern char * text_read_file(char * file_name);
/* ʸ����������������ʤ뤫������ؿ� */
extern int number_syntax(char * c);

void save_suffix(INT * suf, INT size, char *file_name);

int file_exist(char * file);

int file_last_position(char * file);

#ifdef MMAP
char * map_file(char * file_name);
INT file_size(char * file_name);
#endif

