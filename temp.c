//#include "comm.h"
#include <stdlib.h>
#include <string.h>
#define MAX_TABLE_NAME_LEN 128
#define MAX_OP_LEN 8
#define MAX_VARCHAR_LEN 128
#define ERROR -1
typedef enum {
    FALSE = 0,
    TRUE = 1,
}bool_t;
typedef struct {
    bool_t is_int;
    int i;
    char varchar[MAX_VARCHAR_LEN+1];
}int_or_char;

int myisalpha2(char c){
//check the start of a variable, no numbers
	if('a'<=c&&c<='z')
		return 1;
	if('A'<=c&&c<='Z')
		return 1;
	if(c=='_')
		return 1;
	return 0;
}

int requirement(char *s, char a[MAX_TABLE_NAME_LEN], int *b, int_or_char *c){
    int i,space = 0;
    char b_temp[MAX_VARCHAR_LEN];
    char temp[MAX_VARCHAR_LEN];
    if(!myisalpha2(s[0])) return ERROR;
    
    for (i = 0; s[i]!='\0';i++){
    	if(s[i]==' ') space ++;
    }
    
    if (space != 2) return ERROR;
    
    sscanf (s,"%s %s %s",a,b_temp,temp);
    if (temp[0]=='\'') {
    	for (i = 1; temp[i]!='\'';i++){
    	printf("%d\n",i);
    		if (i >= strlen(temp)) return ERROR;
    	}
    	if (temp[i+1]!='\0') return ERROR;
    	c->is_int = 0;
    	sscanf(temp,"%s",c->varchar);
    } else {
    
    	for (i = 0; temp[i]!='\0';i++) {
    		if (temp[i]<'0'||temp[i]>'9')
    			return ERROR;
    	}
    	c->is_int = 1;
    	c->i = atoi(temp);
    }
    if (c->is_int){
    	if(strcmp("<",b_temp)==0) {*b = 1;return 0;}
    	if(strcmp("<=",b_temp)==0) {*b = 2;return 0;}
    	if(strcmp(">",b_temp)==0) {*b = 3;return 0;}
    	if(strcmp(">=",b_temp)==0) {*b = 4;return 0;}
    	if(strcmp("=",b_temp)==0) {*b = 5;return 0;}
    	if(strcmp("!=",b_temp)==0) {*b = 6;return 0;}
    }else {
    	if(strcmp("=",b_temp)==0) {*b = 7;return 0;}
    	if(strcmp("!=",b_temp)==0) {*b = 8;return 0;}
    	if(strcmp("like",b_temp)==0) {*b = 9;return 0;}
    	if(strcmp("not like",b_temp)==0) {*b = 10;return 0;}
    }
    return ERROR;
}

int main(){
	char s[128];
	char a[MAX_TABLE_NAME_LEN];
	int b;
	int_or_char c;
	strcpy(s,"id = 3");
	int res = requirement(s, a,&b,&c);

    printf("%d %d\n",b,res);
	return 0;
}
