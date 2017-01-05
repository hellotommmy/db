//#include "comm.h"
#include <stdio.h>
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

int requirement(char *s){
    int i,space = 0;
    char b_temp[MAX_VARCHAR_LEN];
    char a_temp[MAX_VARCHAR_LEN];
    char temp[MAX_VARCHAR_LEN];
    char c[128];
    if(myisalpha2(s[0])==ERROR) return ERROR;
    
    for (i = 0; s[i]!='\0';i++){
    	if(s[i]==' ') space ++;
    }
    
    if (space != 2) return ERROR;
    
    sscanf (s,"%s %s %s",a_temp,b_temp,temp);
    if (temp[0]=='\'') {
    	for (i = 1; temp[i]!='\'';i++){
    		if (i >= strlen(temp)) return ERROR;
    	}
    	if (temp[i+1]!='\0') return ERROR;
    	temp[i]='\0';
    	sscanf(temp,"'%s",c);
    	printf("%s\n",c);
    }	
    return 0;
}

int main(){
	char s[128];
	sprintf(s,"id = '3'");
	requirement(s);
	return 0;
}

