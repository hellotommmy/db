#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
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

int create(char *table_name, int col, char col_name[][129], int *col_type);
int drop(char *s);
int insert(char *table_name, int col, int_or_char *inchar);
int select_simple(char cols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN], int num, char *table, char *seletcol, int op, int_or_char constant);
int select_join(char cols1[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num1,char *table1,char cols2[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num2,char *table2,char unknowncols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num,char *selectcol1,int op1, int_or_char constant1,char *selectcol2,int op2, int_or_char constant2,char *selectcol3_1,int op3, char *seletcol3_2);


int file_size(char* filename);
void zero(char *buff);
void settypebit(int col, int *col_type, table_head *head);
void page_init(int pagenum, FILE *fp);
bool_t comptypebit(int col, int_or_char *inchar,table_head head);
void readbuff(char *buff, table_head head, int *printbit, int num, int_or_char constant, int op);
int int_op(int a, int b, int op);
int var_op(char *a, char *b, int op);
int buff_write(char *buff, int *varoffset, int *intarry, char *varchararry, int col_num, int intnum);
void buff_init(char *buff);

#endif
