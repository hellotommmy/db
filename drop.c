#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>

#include "comm.h"
int read_which_word_to_t(char *s,char *t,int buffer_len,int which);
int myisalpha(char c);
int parse_drop(char *s,char *t);
void drop_table(char *t);
/*
int main(){
	char s[1000];
	char t[1000];
	int i;
	for(i=0;i<1000;i++){
		s[i] = 0;
		s[i] = 0;
	}
	fgets(s,1000,stdin);
	int result;
	result = parse_drop(s,t);
	printf("command: %s\n",s);
	if(result==OK)
		printf("table name :%s\n",t);
	else
		printf("syntax error\n");
}*/

void drop_table(char *t){
	printf("table %s is dropped\n",t);
}

int myisalpha(char c){
//so called "isalpha"
//if is a letter or a number, return 1
//otherwise return 0
	if('a'<=c&&c<='z')
		return 1;
	if('A'<=c&&c<='Z')
		return 1;
	if('0'<=c&&c<='9')
		return 1;
	if(c=='_')
		return 1;
	return 0;
}
int read_which_word_to_t(char *s,char *t,int buffer_len,int which){
//read a line, the end must be a ;
	int word_count = 0;
	int i = 0 ;
	int j = 0;
	bool_t between_words = TRUE;
	bool_t has_ended = FALSE;
	while(s[i]!=';'){
		if(myisalpha(s[i]))
		{
			if(between_words)
			{
				between_words = FALSE;
				word_count++;
				if(word_count==which)
					t[j++] = s[i];
			}
			else{
				if(word_count==which)
					t[j++] = s[i];
			}
		}
		else if(s[i]==' ')
		{
			if(between_words==FALSE)
				between_words = TRUE;
		}
		else{
			break;
		}
		i++;
		if(j>=buffer_len)
			return ERROR;
	}
	t[j] = 0;
	i++;
	has_ended=TRUE;
	while(s[i]!=0&&s[i]!='\n'){
		if(s[i]!=' '){
			has_ended=FALSE;
			break;
		}
		i++;
	}
	
	if(has_ended)
		return word_count;
	else 
		return ERROR;
}


int parse_drop(char *s,char *t){
	//source s, target t table
	char temp_buffer_drop[10];
	char temp_buffer_table[10];
	sscanf(s,"%9s%9s",temp_buffer_drop,temp_buffer_table);
	if(strcmp(temp_buffer_drop,"drop")!=0)
		return ERROR;
	if(strcmp(temp_buffer_table,"table")!=0)
		return ERROR;
	int word_count;
	char temp_buffer_table_name[MAX_TABLE_NAME_LEN+1];
	word_count = read_which_word_to_t(s,temp_buffer_table_name,MAX_TABLE_NAME_LEN+1,3);//put the third word into temp_buffer_table_name(the table name)
	if(word_count!=3)
		return ERROR;
	else{
		strcpy(t,temp_buffer_table_name);
		return OK;
	}
}
