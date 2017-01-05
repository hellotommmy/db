#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include "queue.h"
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
#define HASHSIZE 101
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
    char col_name[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN];
}table_head;
typedef struct {
    int op;
    char col_name[MAX_TABLE_NAME_LEN];
}aggregation;
typedef struct {
    int res;
    int_or_char groupcol;
}group_by;
typedef struct nlist{
    node_t *next;
    node_t *prev;
    int_or_char a;
    int res[MAX_ITEMS_IN_TABLE];
    int res2[MAX_ITEMS_IN_TABLE];
    int res1[MAX_ITEMS_IN_TABLE];
    int res12[MAX_ITEMS_IN_TABLE];
}nlist;

typedef struct {
    char table[2][MAX_TABLE_NAME_LEN];
    char cols[3][MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN];//select col
    int num_cols[3];//table1,t2,unknown

    int_or_char inchar[2];
    int_or_char amb_inchar[2];
    int amb_join;
    char join[2][MAX_TABLE_NAME_LEN];//join columns
    char filter[2][MAX_TABLE_NAME_LEN];//filter columns
    char amb_filter[2][MAX_TABLE_NAME_LEN];   
    int op[2];//filter and join operation, 0 means no op
    int amb_op[2];
    aggregation agg[3][MAX_ITEMS_IN_TABLE];
    int agg_number[3];
    int which_group;//1-table1,2-table2,3-ambigouos,0-no
    char group_col[MAX_TABLE_NAME_LEN];//history residual
} arg_struct;

nlist *hashtab[HASHSIZE];

int create(char *table_name, int col, char col_name[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN], int *col_type);
int drop(char *s);
int insert(char *table_name, int col, int_or_char *inchar);
int select_simple(char cols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN], int num, char *table, char *selectcol, int op, int_or_char constant);
int select_join(char cols1[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num1,char *table1,char cols2[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num2,char *table2,char unknowncols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num,char *selectcol1,int amb1,int op1, int_or_char constant1,char *selectcol2,int amb2,int op2, int_or_char constant2,char *selectcol3_1,int amb3_1,int op3, char *selectcol3_2,int amb3_2);
int group_simple(char *table, char *groupcol, aggregation *agg, int num, char *selectcol, int op, int_or_char constant);
int group_join(char *table1, char *groupcol1,aggregation *agg1, int num1, char *table2,char *groupcol2,aggregation *agg2, int num2, aggregation *unknown,int num, char *selectcol1, int amb1, int op1,  int_or_char constant1, char *selectcol2, int amb2, int op2, int_or_char constant2,  char *selectcol3_1, int amb3_1, int op3, char *selectcol3_2, int amb3_2);

int writeonepage(int buffnum,char *table_buff, char *buff, table_head head, int *printbit, int num, int_or_char constant, int op);
int writetobuff(char *table_buff, table_head head,FILE *fp, int_or_char constant, int *printbit, int num, int op);

unsigned hash(int_or_char a);
struct nlist *lookup(int_or_char a, aggregation *agg1,aggregation *agg2, int num1,int num2);
struct nlist *install(int_or_char a , aggregation *agg1,aggregation *agg2, int num1,int num2);
int file_size(char* filename);
void zero(char *buff);
void settypebit(int col, int *col_type, table_head *head);
void page_init(int pagenum, FILE *fp);
bool_t comptypebit(int col, int_or_char *inchar,table_head head);
void readbuff(char *buff, table_head head, int *printbit, int num, int_or_char constant, int op);
int int_op(int a, int b, int op);
int var_op(char *a, char *b, int op);
int agg_init (int op);
int aggregation_op (int a, int b, int op);
int buff_write(char *buff, int *varoffset, int *intarry, char *varchararry, int col_num, int intnum);
void buff_init(char *buff);

#endif
