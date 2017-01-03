#ifndef COMM_INCLUDED
#define COMM_INCLUDED
#define BUFFER_LEN 1000
#define MAX_TABLES 100
#define MAX_ITEMS_IN_TABLE PAGE_LEN/MAX_VARCHAR_LEN
#define MAX_VARCHAR_LEN 256
#define PAGE_LEN 4096
#define MAX_TABLE_NAME_LEN 128
#define NUM_RESERVED_WORDS 13
#define MAX_WORD_LEN 32
#define NUM_TYPES 2
#define MAX_RESERVED_WORD_LEN 8//varchar
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

#endif
