#include <stdio.h>
#include "com.h"
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#ifndef COMM_INCLUDED
#define COMM_INCLUDED
#define BUFFER_LEN 1000
#define MAX_TABLES 100
#define MAX_ITEMS_IN_TABLE 100
#define MAX_VARCHAR_LEN 128
#define MAX_TABLE_NAME_LEN 128
#define NUM_RESERVED_WORDS 13
#define MAX_WORD_LEN 32
#define NUM_TYPES 2
#define MAX_RESERVED_WORD_LEN 8//varchar
#define BITNUM 4
#define ONE_PAGE_ITEM 31
#define TWO_PAGE_ITEM 62
#define THREE_PAGE_ITEM 94

#define PAGE_LEN 4096
//#define TRUE 1
//#define FALSE 0
#define ERROR -1
#define OK 1
typedef enum {
	FALSE = 0,
	TRUE = 1,
}bool_t;
typedef struct {
	bool_t is_int;
	int i;
	char varchar[MAX_VARCHAR_LEN+1];
}int_or_char;
typedef struct{
    int col_num;
    int col_type[BITNUM];
    int base;
    int intnum;
    int freepage;
    int datapage;
    short index[MAX_ITEMS_IN_TABLE];
    char col_name[MAX_ITEMS_IN_TABLE][MAX_VARCHAR_LEN];
}table_head;

void settypebit(int col, int *col_type, table_head *head);
void page_init(int pagenum, FILE *fp);
bool_t comptypebit(int col, int_or_char *inchar,table_head head);

#endif
