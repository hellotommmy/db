#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include "comm.h"
int myisalpha1(char c);
int myisalpha1(char c){
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
int myisnumber(char c);
int myisnumber(char c){
	return c<='9'&&c>='0'?1:0;
}
int check_format(char *s);
int check_format(char *s){
	int word_count = 0;
	int i = 0 ;
	bool_t between_words = TRUE;
	while(s[i]!='('){
		if(myisalpha1(s[i]))
		{
			if(between_words)
			{
				between_words = FALSE;
				word_count++;
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
	}
	if(word_count==4)
		return OK;
	else 
		return ERROR;
}
/*
int main(int argc, char const *argv[])
{
	char l[1000];
	int_or_char t[MAX_ITEMS_IN_TABLE+1];
	int cols;
	char table[MAX_TABLE_NAME_LEN+1];

	fgets(l,1000,stdin);
	cols = parse_insert(l,t,table);
	if(cols!=ERROR){
		printf("table name is %s\n",table);
		for (int i = 1; i <= cols; ++i)
		{
			if (t[i].is_int)
				printf("the %dth column is int, value is %d\n",i,t[i].i);
			else
				printf("the %dth column is varchar, value is %s\n",i,t[i].varchar);
		}
	}
	else 
		printf("syntax error!\n");
	return 0;
}*/


int parse_insert(char *s,int_or_char t[MAX_ITEMS_IN_TABLE],char table_name[MAX_TABLE_NAME_LEN]);
int parse_insert(char *s,int_or_char t[MAX_ITEMS_IN_TABLE],char table_name[MAX_TABLE_NAME_LEN]){
	char buffer[BUFFER_LEN];
	int i;
	char grammar[3][10];
	//char test[1000];
	//char table_name[MAX_TABLE_NAME_LEN+1];
	char compare[3][10] = {"insert","into","values"};
	int success;
	success = sscanf(s,"%9s%9s%128s%6s",grammar[0],grammar[1],table_name,grammar[2]);
	//printf("%d\n",success );
	if(success!=4)
		return ERROR;
	if(check_format(s)==ERROR)
		return ERROR;
	if(!(strcmp(grammar[0],compare[0])==0 &&strcmp(grammar[1],compare[1])==0 &&strcmp(grammar[2],compare[2])==0) )
		return ERROR;

	for(i=0;i<BUFFER_LEN;i++){
		buffer[i] = 0;
	}
	if(extract_items_between_brackets_i(s,buffer)==ERROR)
		return ERROR;
	int pieces;
	pieces = cut_into_pieces(buffer,t);
	return pieces;//pieces is ERROR if syntax error

}
int extract_items_between_brackets_i(char *command_buffer,char *info_between_brackets);

int extract_items_between_brackets_i(char *command_buffer,char *info_between_brackets){
	int i=0;
	while(command_buffer[i]!='('){
		if(command_buffer[i]==';')
			return ERROR;
		i++;
	}
	i++;
	int j=0;
	while(command_buffer[i]!=')'){
		if(command_buffer[i]==';'||command_buffer[i]=='(')
			return ERROR;
		info_between_brackets[j++] = command_buffer[i++];
	}
	i++;
	while(command_buffer[i]==' '){
		i++;
	}
	if(command_buffer[i]==';')
		return OK;//happily
	return ERROR;
}

int cut_into_pieces(char *s, int_or_char t[MAX_ITEMS_IN_TABLE+1]);
int cut_into_pieces(char *s, int_or_char t[MAX_ITEMS_IN_TABLE+1]){
	int word_count = 1;
	int i = 0 ;
	int j = 0;
	char state_reg = 0;
	char c;
	char number_buffer[11];//10 bits enough for int
	while(1){
		switch(state_reg){
			case 0:
			if( myisnumber(*(s+i)) ){//if is decimal
				t[word_count].is_int = TRUE;
				state_reg = 1;
				i--;
			}
			else if(*(s+i)=='\''){
				t[word_count].is_int = FALSE;
				state_reg = 2;
			}
			else if(*(s+i)==' ')
				state_reg = 0;
			else
				state_reg = 5;
			i++;
			break;

			case 1:
			if( myisnumber(*(s+i)) ){
				number_buffer[j++] = *(s+i);
				state_reg = 1;
			}
			else if(*(s+i)==','){
				number_buffer[j] = 0;
				j = 0;
				t[word_count].i = atoi(number_buffer);
				word_count++;
				state_reg = 4;
			}
			else if(*(s+i)==' '){
				number_buffer[j] = 0;
				j = 0;
				t[word_count].i = atoi(number_buffer);
				word_count++;				
				state_reg = 3;
			}
			else if(*(s+i)==0){
				number_buffer[j] = 0;
				j = 0;
				t[word_count].i = atoi(number_buffer);
				word_count++;
				state_reg = 6;
			}
			else
				state_reg = 5;
			i++;
			break;

			case 2:
			if(*(s+i)=='\''){
				t[word_count].varchar[j] = 0;
				j = 0;
				word_count++;
				state_reg = 3;
			}
			else if(*(s+i)==0)
				state_reg = 5;
			else
				t[word_count].varchar[j++]=*(s+i);

			i++;
			break;

			case 3:
			if(*(s+i)==' ')
				state_reg = 3;
			else if(*(s+i)==0)
				state_reg = 6;
			else if(*(s+i)==',')
				state_reg = 4;
			else 
				state_reg = 5;
			i++;
			break;
			
			case 4:
			if(*(s+i)==' ')
				state_reg = 4;
			else if(myisnumber(*(s+i))){
				t[word_count].is_int = TRUE;
				i--;
				state_reg = 1;
			}
			else if(*(s+i)=='\''){
				t[word_count].is_int = FALSE;
				state_reg = 2;
			}
			else
				state_reg = 5;
			i++;
			break;
 
			case 5:
			//something wrong has happened
			return ERROR;
			break;

			case 6:
			return word_count-1;
			break;

			default:
			printf("error!!!!!\n");
		}
	}
	return ERROR;
}
