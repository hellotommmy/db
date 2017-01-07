#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include "comm.h"
int number_of_items;
int col_type[MAX_ITEMS_IN_TABLE];//1---int,2---varchar
char col_name[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN];
//column[0]存放表名，column[1],...依次存放各个列名
char reserved[NUM_RESERVED_WORDS][MAX_WORD_LEN]={"create","table","int","varchar","drop","insert","select","from","where","and","or","into","values"};
char type_name[NUM_TYPES][MAX_WORD_LEN]={"int","varchar"};
char command_buffer[BUFFER_LEN];//One request from the client. e.g. create table Demo(id int);
char info_between_brackets[BUFFER_LEN];

//int  parse_drop(char *s,char *t);
//int  parse_drop(char *s,char *t){

//}

//void drop_table();
//void drop_table(){

//}

int read_sql_file_line(FILE * ptr);
void read_create_table(FILE * ptr);
void read_drop_table(FILE * ptr);
void read_select(FILE * ptr);
void read_insert(FILE * ptr);
void process_table_name(char *s);
void clear_buffer();
void do_operation();


//void parse_select();
void extract_items_between_brackets();


void check_name();
int error_flag;
char which_command;
int which_type;//1 stands for create table, 2 for drop, 3 for insert, 4 for select
int command_len;
int main(int argc, char const *argv[])
{
	FILE *fptr;
	//printf("This is the path: %s\n",argv[1]);
	
	fptr = fopen(argv[1],"r");
	if (fptr==NULL)
	{
		printf("open error\n");
		return -1;
	}
	while(read_sql_file_line(fptr)!=-1){
		do_operation();
		//if(which_type!=1)
	//		printf("something unexpected\n");
		error_flag = 0;
		clear_buffer();
	}
	return 0;
}
//clear the command buffer
void check_name(){
	//check if the name contains special characters like '#','$',...
	int i,j;
	for(i=0;i<=number_of_items;i++)
	{
		j=0;
		while(col_name[i][j]){
			if( (col_name[i][j]>='a' && col_name[i][j]<='z') || (col_name[i][j]<='Z' && col_name[i][j]>='A')||
				col_name[i][j]=='_'||(col_name[i][j]>='0'&&col_name[i][j]<='9'&&j>0))
				;
			else{
				error_flag = 1;
				return;
			}
			j++;
		}
	}
}
/*void create_table(){
	int i,j;
	printf("%s\n", col_name[0]);
	for(i=1;i<=number_of_items;i++){
		printf("%s: ",type_name[col_type[i]-1] );
		printf("%s\n",col_name[i]);
	}
	char path_buf[100];
	path_buf[0] = '.';
	path_buf[1] = '/';
	path_buf[2] = 'd';
	path_buf[3] = 'b';
	path_buf[4] = '/';
	i = 0;
	while(col_name[0][i]!=0){
		path_buf[i+5]=col_name[0][i];
		i++;
	}
	path_buf[i+5] = '.';
	path_buf[i+6] = 't';
	path_buf[i+7] = 'b';
	path_buf[i+8] = 'l';
	path_buf[i+9] = '\0';
	fopen(path_buf,"wb+x");
}*/




void clear_buffer(){
	int i,j;
	for(i=0;i<BUFFER_LEN;i++){
		command_buffer[i]=0;
		info_between_brackets[i] = 0;
	}
	for(i=0;i<=MAX_ITEMS_IN_TABLE;i++){
		for(j=0;j<=MAX_VARCHAR_LEN;j++){
			col_name[i][j] = 0;
		}
		col_type[i] = 0;
	}
}

void extract_items_between_brackets(){
	int i=0;
	while(command_buffer[i]!='('){
		if(command_buffer[i]==';'){
			error_flag = 1;
			return ;
		}
		i++;
	}
	i++;
	int j=0;
	while(command_buffer[i]!=')'){
		if(command_buffer[i]==';'||command_buffer[i]=='('){
			error_flag = 1;
			return ;
		}
		info_between_brackets[j++] = command_buffer[i++];
	}
	i++;
	info_between_brackets[j] = 0;
	while(command_buffer[i]==' '){
		i++;
	}
	if(command_buffer[i]==';')
		return;//happily
	error_flag = 1;
}
void extract_col(){
	int i=0;
	int prepare_to_break = 0;
	number_of_items = 1;
	bool_t finished = TRUE;
	char type_buffer[10];
	int j=0;
	while(1){
		while(info_between_brackets[i]==' ')
			i++;
		if(info_between_brackets[i]==0){
			error_flag = 1;//two cases:() or (hello int,)
			break;
		}
		j = 0;
		while(info_between_brackets[i]!=' '){
			if(info_between_brackets[i]==0)
				{error_flag = 1; break;}
			col_name[number_of_items][j++] = info_between_brackets[i++];
		}
		col_name[number_of_items][j]=0;
		//判断列名是否与关键字冲突
		for(j=0;j<NUM_RESERVED_WORDS;j++){
			if(strcmp(reserved[j],col_name[number_of_items])==0){
				error_flag = 1;
				return;
			}
		}
		finished = FALSE;
		while(info_between_brackets[i]==' ')
			i++;
		if(info_between_brackets[i]==0)
			{error_flag = 1; break;}	
		j = 0;
		while(info_between_brackets[i]!=' '&&info_between_brackets[i]!=','&&j<10){
			if (info_between_brackets[i]==0)
			{
				prepare_to_break = 1;
				break;
			}
			type_buffer[j++] =  info_between_brackets[i++];

		}
		if(j!=3 && j!=7){
			error_flag = 1;
			return ;
			}
		type_buffer[j]=0;
		//char type_name[NUM_TYPES][MAX_WORD_LEN]={"int","varchar"};
		for(j=0;j<NUM_TYPES;j++){
			if(strcmp(type_buffer,type_name[j])==0){
				col_type[number_of_items] = j+1;
				break;	
			}	
		}
		if(j==NUM_TYPES)//出现int,varchar以外的关键字
		{error_flag=1;return;}
		if (prepare_to_break)
		{
			break;
		}
		if(info_between_brackets[i++]==',')
			finished = TRUE;
		else{
			while(info_between_brackets[i]==' ')
				i++;
			if(info_between_brackets[i]==','){
				i++;
				finished = TRUE;
			}
			else if(info_between_brackets[i]==0)
				return;
			else{
				error_flag = 1;
				return;
			}
		}
		if(finished==FALSE){
			error_flag = 1;
			return;
			}
		number_of_items++;
		if(number_of_items>MAX_ITEMS_IN_TABLE)
		{
			error_flag = 1;
			return ;
		}
	}	
}


int number_of_lines;
void do_operation(){
	//have a line of operation, decide what to do
	char table_name[MAX_TABLE_NAME_LEN];
	int_or_char inchar[MAX_ITEMS_IN_TABLE];
	int cols;
	char middle_buffer[6][1000];
	char sign_flag[6];
	int check_format;
	arg_struct O;
	int i;
//	int k;
	switch(which_type){
	case 1:  
		parse_create(command_buffer,&error_flag);
		if(error_flag==0){
			check_name();
			if(error_flag==1){
				printf("Syntax error\n");
				return;
			}
//			printf("col name:%s\n",col_name[0] );
//			printf("col items:\n");
//			for(i=1;i<=number_of_items;i++){
//				printf("%s,type is %d\n",col_name[i],col_type[i] );
//			}
			create(col_name[0],number_of_items,&(col_name[1]),&(col_type[1]));
		}
		else if(error_flag==2)
			printf("Can’t create table\n");
		else{
			printf("Syntax error\n");
			}
		break;
	case 2:
		if(parse_drop(command_buffer,col_name[0])==OK)
			drop(col_name[0]);
		else
			printf("Syntax error\n");
		break;
	case 3:
	//printf("%s\n",command_buffer );
		cols = parse_insert(command_buffer,inchar,table_name);
		//printf("cols:%d\n",cols );
		if(cols==ERROR){
			//printf("Syntax error1\n");
			printf("Syntax error:%d\n",number_of_lines);
		}
		else{
			insert(table_name,cols,&(inchar[1]));
		}
		break;
	case 4:
	check_format = parse_select(&O,command_buffer,middle_buffer,sign_flag);
	if(check_format==ERROR){
		printf("Syntax error\n");
		break;
		}
	if(O.table_number==1){
		if(O.agg_number[0]!=0||O.agg_number[1]!=0||O.agg_number[2]!=0||O.which_group!=0){
			//1 table yes agg
			
				check_format=
				group_simple(
				O.table[0],
				O.agg[0][0].col_name,
				O.agg[0],
				O.agg_number[0],
				O.filter[0],
				O.op[0],
				O.inchar[0]);

				
	
		}
		else{
			check_format=
			select_simple(
				O.cols[0],
				O.num_cols[0],
				O.table[0],
				O.filter[0],
				O.op[0],
				O.inchar[0]
				);
	
		}
	}
	else{
		if(O.agg_number[0]!=0||O.agg_number[1]!=0||O.agg_number[2]!=0){
			//aggregation appears
				if(O.filter[0][0]!=0&&O.filter[1][0]!=0)
				check_format=group_join(O.table[0],O.agg[0][0].col_name,
				O.agg[0],O.agg_number[0],
				O.table[1],O.agg[1][0].col_name,
				O.agg[1],O.agg_number[1],
				O.agg[2],O.agg_number[2],

				 O.filter[0],0//no ambi
				,O.op[0],O.inchar[0],

				 O.filter[1],0,
				 O.op[1],O.inchar[1],//no ambi

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="
			else if(O.filter[0][0]!=0&&O.filter[1][0]==0)
				check_format=group_join(O.table[0],O.agg[0][0].col_name,
				O.agg[0],O.agg_number[0],
				O.table[1],O.agg[1][0].col_name,
				O.agg[1],O.agg_number[1],
				O.agg[2],O.agg_number[2],

				 O.filter[0],0//no ambi
				,O.op[0],O.inchar[0],

				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				 O.amb_op[0],O.amb_inchar[0],

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="

			else if(O.filter[0][0]==0&&O.filter[1][0]!=0)
				check_format=group_join(O.table[0],O.agg[0][0].col_name,
				O.agg[0],O.agg_number[0],
				O.table[1],O.agg[1][0].col_name,
				O.agg[1],O.agg_number[1],
				O.agg[2],O.agg_number[2],

				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				O.amb_op[0],O.amb_inchar[0],

				 O.filter[1],0,
				 O.op[1],O.inchar[1],//no ambi

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="
			else
				check_format=group_join(O.table[0],O.agg[0][0].col_name,
				O.agg[0],O.agg_number[0],
				O.table[1],O.agg[1][0].col_name,
				O.agg[1],O.agg_number[1],
				O.agg[2],O.agg_number[2],

				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				O.amb_op[0],O.amb_inchar[0],

				 O.amb_filter[1],O.amb_op[1]!=0,//is ambi
				 O.amb_op[1],O.amb_inchar[1],

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="
			//check result
	
				
		}
		else{
		//no aggs
			if(O.filter[0][0]!=0&&O.filter[1][0]!=0)
				check_format=select_join(
				O.cols[0],
				O.num_cols[0],
				O.table[0],

				O.cols[1],
				O.num_cols[1],
				O.table[1],

				O.cols[2],
				O.num_cols[1],
				
				O.filter[0],0,//no ambi
				O.op[0],O.inchar[0],

				 O.filter[1],0,
				 O.op[1],O.inchar[1],//no ambi

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="
			else if(O.filter[0][0]!=0&&O.filter[1][0]==0)
				check_format=select_join(
				O.cols[0],
				O.num_cols[0],
				O.table[0],

				O.cols[1],
				O.num_cols[1],
				O.table[1],

				O.cols[2],
				O.num_cols[1],
				
				O.filter[0],0,//no ambi
				O.op[0],O.inchar[0],

				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				 O.amb_op[0],O.amb_inchar[0],

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="

			else if(O.filter[0][0]==0&&O.filter[1][0]!=0)
				check_format=select_join(
				O.cols[0],
				O.num_cols[0],
				O.table[0],

				O.cols[1],
				O.num_cols[1],
				O.table[1],

				O.cols[2],
				O.num_cols[1],
				
				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				 O.amb_op[0],O.amb_inchar[0],

				 O.filter[1],0,
				 O.op[1],O.inchar[1],//no ambi

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="

			else{
				check_format=select_join(
				O.cols[0],
				O.num_cols[0],
				O.table[0],

				O.cols[1],
				O.num_cols[1],
				O.table[1],

				O.cols[2],
				O.num_cols[1],
				
				 O.amb_filter[0],O.amb_op[0]!=0,//is ambi
				 O.amb_op[0],O.amb_inchar[0],

				 O.amb_filter[1],O.amb_op[1]!=0,//is ambi
				 O.amb_op[1],O.amb_inchar[1],

				O.join[0],O.amb_join,
				1,O.join[1],O.amb_join);//1 means "="
			}

		
		}
	}
	break;
	default:
		break;
	}
	//printf("%s\n",command_buffer);
}

//read a valid command into command buffer, i.e. not an explanation
int read_sql_file_line(FILE * ptr){//read one line in a .sql file
	int state_reg;
	int is_EOF;
	int leaving_flagn = 1;
	char waste[1000];
	state_reg=0;
	int i;
	number_of_lines++;
	bool_t in_varchar=FALSE;
	while(leaving_flagn){
	is_EOF=fscanf(ptr,"%c",&which_command);
	//if(number_of_lines==115)
		//printf("%c",which_command );
	if(is_EOF==EOF)
		return -1;
	switch(state_reg){
		case 0:
			switch(which_command){
				case '\r':
					state_reg = 0;
					break;
				case ' ':
					state_reg = 0;
					break;
				case '\n':
					state_reg = 0;
					break;
				case 'c':
					state_reg = 3;
					which_type = 1;
					break;
				case 'd':
					state_reg = 3;
					which_type = 2;
					break;
				case 'i':
					state_reg = 3;
					which_type = 3;
					break;
				case 's':
					state_reg = 3;
					which_type = 4;
					break;
				case '-':
					state_reg = 2;
					break;
				default :
					state_reg = 0;
				//	printf("in varchar?:%d\n",in_varchar );
				//	printf("%d\n",number_of_lines );
					printf("Syntax error4,char = %c\n",which_command);
					fgets(waste,1000,ptr);
			}
			break;
		case 2:
			if(which_command=='-')
				state_reg = 5;
			else{
				state_reg = 0 ;
				printf("Syntax error5\n");
				fgets(waste,1000,ptr);
				}
			break;
		case 3:
					//printf("char before:%c\n",which_command );
			fseek(ptr,-2,1);
			i=0;
					//printf("char after:%c\n",which_command );
			state_reg = 4;
			break;
		case 4:
			command_buffer[i] = which_command;	
			if(which_command=='\''){
				if(in_varchar==TRUE)
					in_varchar=FALSE;
				else
					in_varchar=TRUE;
			}

			if(which_command==';'){
				if(in_varchar==TRUE){
					state_reg=4;
				}
				else{
					command_len = i;
					leaving_flagn = 0;
					state_reg = 0;
				}
			}
			else if(which_command==0){
				printf("Syntax error6\n");
				state_reg = 2;
				clear_buffer();
			}
			i++;
			break;
		case 5:
			if(which_command=='\n')
				state_reg = 0;
			else
				state_reg = 5;
			break;
		default:
			printf("unknown error\n");
		}
	}
	return 0;
}

void process_table_name(char *s)
{
	int i=0;
	char c=0;
	char table_name[MAX_TABLE_NAME_LEN+1];
	c=*(s+i);
	while(c!=0){
		c=*(s+i);
		table_name[i]=c;
		i++;
		if(i==MAX_TABLE_NAME_LEN){
			printf("NOTE: table name will be truncated into %s\n",table_name);
			break;
		}
	}
	table_name[i]='\0';
	//check table name: same name exists?
    DIR * dir;
    struct dirent * dir_ptr;
    char file_list[MAX_TABLES][40];
    dir = opendir("./db"); //打开db目录
    while((dir_ptr = readdir(dir)) != NULL) //循环读取目录数据
    {
        strcpy(file_list[i],dir_ptr->d_name ); //存储到数组
        if ( ++i>=100 ) break;
    }
    int j;
    char *p;
    //p=(char *)malloc(100);
    for(j=0;j<i;j++){
    	p=strtok(file_list[j],".");
    	if(p){//if 用于防止.,..的情况
    		if(strcmp(p,table_name)==0){
    			error_flag=2;
    			return;
    		}
    	}
    }
    closedir(dir);//关闭目录指针
    //check table name:name is contradictory to reserved names such as table, create,...?
    for(j=0;j<NUM_RESERVED_WORDS;j++){
    	if(strcmp(reserved[j],table_name)==0){
    		error_flag=2;
    		return;
    	}
    }
    //put the table name in the argument array
    sprintf(col_name[0],"%s",table_name);
    error_flag=0;
}
