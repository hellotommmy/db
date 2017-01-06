
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "comm.h"

void parse_create(char *command_buffer,int *error_flag);
void create_table(int col,char s[MAX_ITEMS_IN_TABLE+1][MAX_VARCHAR_LEN+1]);
void parse_create(char *command_buffer, int *error_flag){
	char temp_buffer_create[10];
	char temp_buffer_table[10];
	char temp_buffer_table_name[MAX_TABLE_NAME_LEN+1];
	char temp_check_if_bracket_error[100];
		//printf("%s",command_buffer);
	sscanf(command_buffer,"%9s%9s%128s",temp_buffer_create,temp_buffer_table,temp_buffer_table_name);//用这种方法可以限定只读入一个不超过32byte的字符串（遇到空格就停，绝不会多读）

	if(strcmp(temp_buffer_create,"create")!=0||strcmp(temp_buffer_table,"table")!=0){
		*error_flag = 1;
		return;
		}
	process_table_name(temp_buffer_table_name);//注意还要处理保证输入没有特殊字符比如# ￥之类的
	if(*error_flag!=0)
		return;
	//读每一个域
	//把括号之间的内容提取出来（包括空格之类的）
	extract_items_between_brackets();
	if(*error_flag!=0)
		return;
	extract_col();
}

