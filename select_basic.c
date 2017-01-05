/*
+int select_join(
+ char cols1[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],     //第一个表的列
+ int num1,                                               //第一个表的列的个数 （num1和num2同时为0 表示select*）
+ char *table1,                                           //第一个表名
+ char cols2[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],     //第二个表的列
+ int num2,                                               //第二个表的列的个数
+ char *table2,                                           //的二个表名
+ char unknowncols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],//未知的列
+ int num,                                                //未知的列的个数

+ char *seletcol1,                                        //选择条件1的列
+ int op1,                                                //选择条件1的操作（同上的op编码，op=0表示无此条件）
+ int_or_char constant1,                                  //选择条件1的常量
+ char *seletcol2,                                        //选择条件2的列
+ int op2,                                                //选择条件2的操作（同上的op编码，op=0表示无此条件）
+ int_or_char constant2,                                  //选择条件2的常量
+ char *seletcol3_1,                                      //连接条件的第一个列（来自table1）
+ int op3,                                                //连接条件的操作（同上的op编码，op=0表示无此条件）
+ char *seletcol3_2                                       //连接条件的第而个列（来自table2）
+ )
+ */
#include <string.h>
#include <stdio.h>
#include "comm.h"
int format_check(char *s,char middle_buffer[6][1000],char sign_flag[6]);
int myisalpha2(char c);
int myisalpha3(char *c);
int myisalpha4(char *c);
int check_and(char *c);
int check_scolon(char *s);
int parse_select(arg_struct *O,char *s,char middle_buffer[6][1000],char sign_flag[6]);
int group_check(char * ,arg_struct *O,int mode);
int parse_select_begin(char middle_buffer[6][1000],char sign_flag[6],arg_struct *O);
int filter(arg_struct *O,char *s,int mode);
int isnumber(char c);
int myisalpha1(char c);
int cut(int *amb_join,char *table1,char *table2,char *s,char col[MAX_VARCHAR_LEN],int *op,char constant[MAX_VARCHAR_LEN]);
int cut(int *amb_join,char *table1,char *table2,char *s,char col[MAX_VARCHAR_LEN],int *op,char constant[MAX_VARCHAR_LEN]){
	//return 0 for 1 table
	//return 1 for table 1 filtering
	//2 for table 2
	//3 for ambiguous col
	//4 for join
	//-1 for error
	int i=0;
	int j=0;
	if(table2[0]==0)
		goto one_table;

	while(s[i]==' ')
		i++;
	if(!myisalpha2(s[i]))
		return ERROR;
	int dot_flag[2]={0};
	while(s[i]!='<'&&s[i]!='>'&&s[i]!='='&&s[i]!=' '){
		if(s[i]=='.'){
			dot_flag[0]=i;
			i++;
			col[j]=0;
			break;
		}
		if(!(myisalpha1(s[i])))
			return ERROR;
		col[j++]=s[i++];
	}
	while(s[i]==' ')
		i++;
	if(s[i]=='.'){
		col[j]=0;
		dot_flag[0]=i;
		i++;
		while(s[i]==' ')
			i++;
	}	
	int which_table[2]={0};
	if(0!=dot_flag[0]){
		which_table[0]=(strcmp(table1,col)==0);
		which_table[1]=(strcmp(table2,col)==0);
		if(which_table[0]==0&&which_table[1]==0)
			return ERROR;
		if(!myisalpha1(s[i]))
			return ERROR;
		j=0;
		while(s[i]!=' '&&s[i]!='<'&&s[i]!='='&&s[i]!='>'){
			if(myisalpha1(s[i])==0)
				return ERROR;
			if(which_table[0])//table 1's filter or join
				col[j++]=s[i++];
			else
				constant[j++]=s[i++];//table 2's filter or join
		}
		if(which_table[0])
			col[j]=0;
		else
			constant[j]=0;	
	}
	else{
		col[j]=0;
	}
	int small_flag=0;
	int big_flag=0;
	int equal_flag=0;
	int like_flag=0;
	int not_like_flag=0;
	int not_equal_flag=0;
	while(s[i]==' ')
		i++;
	while(s[i]!=' '){
		switch(s[i]){
			case '!':
			not_equal_flag=1;
			if(s[i+1]!='=')
				return ERROR;
			i+=2;
			goto outside_while;
			case '<':
			if(small_flag||big_flag||equal_flag)
				return ERROR;
			i++;
			small_flag=1;
			break;
			case '=':
			equal_flag=1;
			if(small_flag&&big_flag)
				return ERROR;
			i++;
			goto outside_while;
			break;
			case '>':
			if(small_flag||big_flag||equal_flag)
				return ERROR;
			i++;
			big_flag=1;
			break;
			case 'l':
			if(s[i+1]=='i'&&s[i+2]=='k'&&s[i+3]=='e'&&s[i+4]==' ')
				i+=5;
			else
				return ERROR;
			like_flag=1;
			goto outside_while;				
			break;
			case 'n':
			if(s[i+1]=='o'&&s[i+2]=='t'&&s[i+3]==' '){				
				i+=4;
				while(s[i]==' ')
					i++;
				if(s[i]=='l'&&s[i+1]=='i'&&s[i+2]=='k'&&s[i+3]=='e'&&s[i+4]==' ')
				{
					i+=5;
					not_like_flag=1;
					goto outside_while;
				}
				else
					return ERROR;
			}
			else
				return ERROR;
			return ERROR;
			default:
			return ERROR;			
		}
	}
	outside_while:
	while(s[i]==' ')
		i++;
	j=0;
	if(like_flag==1){
		if(big_flag||small_flag)
			return ERROR;
		*op=9;
		if(s[i]!='\'')
			return ERROR;
		i++;
		while(s[i]!='\''){
			if(s[i]==0)
				return ERROR;			
			if(which_table[1])
				col[j++]=s[i++];
			else
				constant[j++]=s[i++];
		}
		if(which_table[1]){
			col[j]=0;
			return 2;
		}
		else if(which_table[0]){
			constant[j]=0;
			return 1;
		}
		else{
			constant[j]=0;
			return 3;
		}
	}
	if(not_like_flag==1){
		if(big_flag||small_flag)
			return ERROR;
		*op=10;
		if(s[i]!='\'')
			return ERROR;
		i++;
		while(s[i]!='\''){
			if(s[i]==0)
				return ERROR;			
			if(which_table[1])
				col[j++]=s[i++];
			else
				constant[j++]=s[i++];
		}
		if(which_table[1]){
			col[j]=0;
			return 2;
		}
		else if(which_table[0]){
			constant[j]=0;
			return 1;	
		}
		else{
			constant[j]=0;
			return 3;
		}
	}
	if(equal_flag){
		if(small_flag){
			//<=
			*op=2;
			while(s[i]!=' '&&s[i]!=0)
			{
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else if(big_flag){
			//>=
			*op=4;
			while(s[i]!=' '&&s[i]!=0)
			{
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else{
			//real equal
			//check if is string
			if(myisalpha2(s[i])){
				goto join;
			}
			else if(s[i]=='\''){
				i++;
				*op=7;
				while(s[i]!='\''){
					if(s[i]==0)
						return ERROR;			
					if(which_table[1])
						col[j++]=s[i++];
					else
						constant[j++]=s[i++];
				}
				if(which_table[1]){
					col[j]=0;
					return 2;
				}
				else if(which_table[0]){
					constant[j]=0;
					return 1;
				}
				else{
					constant[j]=0;
					return 3;
				}
			}
			else{
				if(isnumber(s[i])){
					*op=5;
					while(s[i]!='\0'&&s[i]!=' '){
						if(!(isnumber(s[i])))
							return ERROR;			
						if(which_table[1])
							col[j++]=s[i++];
						else
							constant[j++]=s[i++];
					}
					if(which_table[1]){
						col[j]=0;
						return 2;
					}
					else if(which_table[0]){
						constant[j]=0;
						return 1;
					}
					else{
						constant[j]=0;
						return 3;
					}
				}
				else
					return ERROR;
			}
		}
	}
	if(not_equal_flag){
		if(s[i]=='\''){
			i++;
			*op=8;
			while(s[i]!='\''){
				if(s[i]==0)
					return ERROR;			
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else if(isnumber(s[i])){
			*op=6;
			while(s[i]!='\0'&&s[i]!=' '){
				if(!(isnumber(s[i])))
					return ERROR;			
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else
			return ERROR;
	}
	if(small_flag){
		//<
		*op=1;
		if(isnumber(s[i])){
			while(s[i]!='\0'&&s[i]!=' '){
				if(!(isnumber(s[i])))
					return ERROR;			
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else
			return ERROR;
	}
	if(big_flag){
		*op=3;
		if(isnumber(s[i])){
			while(s[i]!='\0'&&s[i]!=' '){
				if(!(isnumber(s[i])))
					return ERROR;			
				if(which_table[1])
					col[j++]=s[i++];
				else
					constant[j++]=s[i++];
			}
			if(which_table[1]){
				col[j]=0;
				return 2;
			}
			else if(which_table[0]){
				constant[j]=0;
				return 1;
			}
			else{
				constant[j]=0;
				return 3;
			}
		}
		else
			return ERROR;		
	}
	return 0;
	join:
	j=0;
	if(which_table[0]==1&&which_table[1]==1)
		return ERROR;
	if(which_table[0]==1){
		//table 1 is chosen
		//need to choose table 2
		while(s[i]!=' '&&s[i]!='\0'){
			if(s[i]=='.'){
				dot_flag[1]=i;
				i++;
				constant[j]=0;
				break;
			}
			if(!(myisalpha1(s[i])))
				return ERROR;
			constant[j++]=s[i++];
		}
		while(s[i]==' ')
			i++;
		if(s[i]=='.'){
			constant[j]=0;
			dot_flag[1]=i;
			i++;
			while(s[i]==' ')
				i++;
		}	
		if(0!=dot_flag[1]){
			which_table[1]=(strcmp(table2,constant)==0);
			if(which_table[1]==0)
				return ERROR;
			if(!myisalpha1(s[i]))
				return ERROR;
			j=0;
			while(s[i]!=' '&&s[i]!=0){
				if(myisalpha1(s[i])==0)
					return ERROR;
				constant[j++]=s[i++];//table 2's filter or join
			}
		}
		constant[j]=0;
		return 4;
	}
	else if(which_table[1]){
		//table 2 is chosen
		//need to choose table 1
		while(s[i]!=' '&&s[i]!='\0'){
			if(s[i]=='.'){
				dot_flag[1]=i;
				i++;
				col[j]=0;
				break;
			}
			if(!(myisalpha1(s[i])))
				return ERROR;
			col[j++]=s[i++];
		}
		while(s[i]==' ')
			i++;
		if(s[i]=='.'){
			col[j]=0;
			dot_flag[1]=i;
			i++;
			while(s[i]==' ')
				i++;
		}	
		if(0!=dot_flag[1]){
			//if table name specified, should be the same with table1
			which_table[0]=(strcmp(table1,col)==0);
			if(which_table[0]==0)
				return ERROR;
			if(!myisalpha1(s[i]))
				return ERROR;
			j=0;
			while(s[i]!=' '&&s[i]!=0){
				if(myisalpha1(s[i])==0)
					return ERROR;
				col[j++]=s[i++];//table 2's filter or join
			}
		}
		col[j]=0;
		return 4;		
	}
	else{
		int match;
		while(s[i]!=' '&&s[i]!='\0'){
			if(s[i]=='.'){
				dot_flag[1]=i;
				i++;
				constant[j]=0;
				break;
			}
			if(!(myisalpha1(s[i])))
				return ERROR;
			constant[j++]=s[i++];
		}
		while(s[i]==' ')
			i++;
		if(s[i]=='.'){
			constant[j]=0;
			dot_flag[1]=i;
			i++;
			while(s[i]==' ')
				i++;
		}	
		if(0!=dot_flag[1]){
			if(strcmp(table1,constant)==0)
				match = 1;
			else{
				if(strcmp(table2,constant)!=0)
					*amb_join=1;
				match = 2;
			}
			if(match==2){
				if(!myisalpha1(s[i]))
					return ERROR;
				j=0;
				while(s[i]!=' '&&s[i]!=0){
					if(myisalpha1(s[i])==0)
						return ERROR;
					constant[j++]=s[i++];//table 2's filter or join
				}
			}
			else{
				//match==1,need to shift 2 table's orders
				//char temp[MAX_VARCHAR_LEN];
				//strcpy(temp,col);
				strcpy(constant,col);
				if(!myisalpha1(s[i]))
					return ERROR;
				j=0;
				while(s[i]!=' '&&s[i]!=0){
					if(myisalpha1(s[i])==0)
						return ERROR;
					col[j++]=s[i++];//table 2's filter or join
				}
				col[j]=0;
				return 4;				
			}
		}
		constant[j]=0;
		return 4;
	}
	one_table:
	i=0;
	while(s[i]!='.'){
		i++;
		if(s[i]==0)
			break;
	}
	if(s[i]==0){
		//no col_name
		i=0;
		while(s[i]!=0&&s[i]!=' ' )
			{col[i]=s[i];i++;}
		s[i]=0;
	}
	else{
		j=0;
		i++;
		while(s[i]==' ')
			i++;
		while(s[i]!=' '&&s[i]!=0)
			col[j++]=s[i++];
		col[j]=0;
	}
	return 1;
}

int myisalpha1(char c){
//check the start of a variable, no numbers
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
int isnumber(char c){
	if('0'<=c&&c<='9')
		return 1;
	else
		return 0;	
}
int filter(arg_struct *O,char *s,int mode){
	int status;
	int op;
	int amb_join=0;
	char col_buff[MAX_TABLE_NAME_LEN];
	char constant_buff[MAX_VARCHAR_LEN];
	if(mode==2){
		status=cut(&amb_join,O->table[0],O->table[1],s,col_buff,&op,constant_buff);
		if(status==ERROR)
			return ERROR;
		if(status==4){
			O->amb_join = amb_join;
			strcpy(O->join[0],col_buff);
			strcpy(O->join[1],constant_buff);
		}
		else if(status==3){
			if(O->amb_op[0]){
				O->amb_op[0]=op;
				strcpy(O->amb_filter[0],col_buff);
				if(op>=1&&op<=6){
					//int
					O->amb_inchar[0].is_int=TRUE;
					O->amb_inchar[0].i=atoi(constant_buff);
				}
				else{
					O->amb_inchar[0].is_int=FALSE;
					strcpy(O->amb_inchar[0].varchar,constant_buff);
				}				
			}
			else{
				O->amb_op[1]=op;
				strcpy(O->amb_filter[1],col_buff);
				if(op>=1&&op<=6){
					//int
					O->amb_inchar[1].is_int=TRUE;
					O->amb_inchar[1].i=atoi(constant_buff);
				}
				else{
					O->amb_inchar[1].is_int=FALSE;
					strcpy(O->amb_inchar[1].varchar,constant_buff);
				}	
			}
		}
		else{
			O->op[status-1]=op;
			strcpy(O->filter[status-1],col_buff);
			if(op>=1&&op<=6){
				//int
				O->inchar[status-1].is_int=TRUE;
				O->inchar[status-1].i=atoi(constant_buff);
			}
			else{
				O->inchar[status-1].is_int=FALSE;
				strcpy(O->inchar[status-1].varchar,constant_buff);
			}
		}	
	}
	if(mode==1){
		if(O->op[0]||O->op[1]||O->op[2])//only 1 col should exist
			return ERROR;
		status = cut(&amb_join,O->table[0],O->table[1],s,col_buff,&op,constant_buff);
		if(status==ERROR||status==4)
			return ERROR;
		O->op[0]=op;
		strcpy(O->filter[0],col_buff);
		if(op>=1&&op<=6){
			//int
			O->inchar[0].is_int=TRUE;
			O->inchar[0].i=atoi(constant_buff);
		}
		else{
			O->inchar[0].is_int=FALSE;
			strcpy(O->inchar[0].varchar,constant_buff);
		}
		return OK;
	}
}

int group_check(char *s ,arg_struct *O,int mode){
	if(mode==2){
		if(O->which_group==0)
			return ERROR;
		int i=0,j=0;
		char buffer[MAX_TABLE_NAME_LEN];
		memset(buffer,0,MAX_TABLE_NAME_LEN);
		while(s[i]==' ')
			i++;
		if(!myisalpha2(s[i]))
			return ERROR;
		while(s[i]!=' '&&s[i]!=','&&s[i]!='.'&&s[i]!=0){
			buffer[j++]=s[i++];
		}
		buffer[j]=0;
		while(s[i]==' ')
			i++;
		if(s[i]==',')
			goto need_more_info;
		if(s[i]==0)
			goto need_more_info;
		i++;
		while(s[i]==' ')
			i++;
		buffer[j]='.';
		j++;
		while(s[i]!=' '&&s[i]!=0)
			buffer[j++]=s[i++];
		int result;
		result=which_table(O->table[0],O->table[1],buffer)+1;
		if(result==4||result==ERROR)
			return ERROR;
		if(result==3)
			goto need_more_info;
		else{
			if(O->which_group!=3){
				//if which group = result, not 3, nothing to do
				if(result!=O->which_group)
					return ERROR;
				else
					return result;
			}
			else{
				O->which_group=result;
				strcpy(O->agg[O->which_group][0].col_name,O->agg[2][0].col_name);
				return result;
			}
		}
		need_more_info:
		if(O->which_group==3)
			goto final_ambig;
		else
			return O->which_group;
		final_ambig:
		//no need to do operation since have ended
		return 3;
	}
	else if(mode==1){
		//default 1 table
		if(O->which_group==0)
			return ERROR;
		if(strcmp(O->group_col,s)!=0)
			return ERROR;
		strcpy(O->agg[0][0].col_name,O->group_col);
		return OK;
	}
	else
		return ERROR;
}
int which_table(char *table1,char *table2,char *query){
	//return 0 if table1, 1 if table 2,2 if unknown
	//meanwhile, cut table name from query
	//e.g table1=te1,table2=t2;query=t2 . id
	//return 1, and query=id after return
	//return ERROR if query format wrong, e.g t.a.d, t#d
	//3 if *
	int i=0;	
	while(query[i]==' ')
		i++;
	if(query[0]=='*')
		return 3;
	int first_space;
	int first_dot;
	if(!myisalpha2(query[0]))
		return ERROR;
	while(query[i]!='.'&&query[i]!=' ')
	{
		if(query[i]==0)
			return 2;
		if(!myisalpha(query[i]))
			return ERROR;
		i++;
	}
	first_space=i;
	while(query[i]==' ')
		i++;
	 first_dot=i;
	 char temp;
	 int j;
	if(query[i]=='.'){
		temp=query[first_space];
		query[first_space]=0;
		if(strcmp(table1,query)==0){
			query[first_space]=temp;
			i++;
			while(query[i]==' ')
				i++;
			if(query[i]==0)
				return ERROR;
			j=0;
			while(query[i]!=' '&&query[i]!=0)
				query[j++]=query[i++];
			query[j]=0;
			while(query[i]==' ')
				i++;
			if(query[i]!=0)
				return ERROR;
			return 0;
		}
		if(strcmp(table2,query)==0){
			query[first_space]=temp;
			i++;
			while(query[i]==' ')
				i++;
			if(query[i]==0)
				return ERROR;
			j=0;
			while(query[i]!=' '&&query[i]!=0)
				query[j++]=query[i++];
			query[j]=0;
			while(query[i]==' ')
				i++;
			if(query[i]!=0)
				return ERROR;
			return 1;
		}
		return ERROR;//no matching of 2 tables with a .
	}
	else if(query[i]==0)
	{
		query[first_space]=0;
		return 2;
	}

		return ERROR;
}

int how_many_tb(char *s,arg_struct *O);
int extract_col1(arg_struct * O,char *s);
int extract_col2(arg_struct * O,char *s);
int extract_col2(arg_struct * O,char *s){
//	table[1]=O->table[0];
//	table[2]=O->table[1];
	int i,j,k;
	char buffer[MAX_TABLE_NAME_LEN];
	O->which_group=0;
	char table_name_buffer[MAX_TABLE_NAME_LEN];
	int simple_col_number[3]={0};
	int agg_col_number[3]={0};
	int table_res;
	if(!myisalpha2(s[0])){
		if(s[0]=='*'&&s[1]==0){
			goto finish;
		}
		return ERROR;
	}
	int op =0 ;
	general:
	op = 0;
	while(s[i]==' ')
		i++;
	if(s[i]=='s'&&s[i+1]=='u'&&s[i+2]=='m'&&(s[i+3]==' '||s[i+3]=='('))
	{	
		op=1;
		goto agg_check;
	}

	if(s[i]=='c'&&s[i+1]=='o'&&s[i+2]=='u'&&s[i+3]=='n'&&s[i+4]=='t'&&(s[i+5]==' '||s[i+5]=='('))
	{
		op=2;
		goto agg_check;
	}		
	if(s[i]=='a'&&s[i+1]=='v'&&s[i+2]=='g'&&(s[i+3]==' '||s[i+3]=='('))	
	{
		op=3;
		goto agg_check;
	}		
	if(s[i]=='m'&&s[i+1]=='i'&&s[i+2]=='n'&&(s[i+3]==' '||s[i+3]=='('))	
	{
		op=4;
		goto agg_check;
	}		
	if(s[i]=='m'&&s[i+1]=='a'&&s[i+2]=='x'&&(s[i+3]==' '||s[i+3]=='('))	
	{
		op=5;
		goto agg_check;
	}
	if(!myisalpha2(s[i]))
		return ERROR;
	//goto simple_check;
	//simple_check:
	if(O->which_group!=0)//group by col exists
		return ERROR;
	else if(agg_col_number[0]+agg_col_number[1]+agg_col_number[2]>=1){
		//group by col
		j=0;
		memset(buffer,0,MAX_TABLE_NAME_LEN);
		while(s[i]!='.'&&s[i]!=' '&&s[i]!=0&&s[i]!=',')
			buffer[j++]=s[i++];
		while(s[i]==' ')
			i++;
		if(s[i]==0){
			buffer[j]=0;
			strcpy(O->agg[2][0].col_name,buffer);//2-unknown col
			O->which_group=3;
			goto finish;
		}
		if(s[i]=='.'){
			buffer[j++]=s[i++];
			while(s[i]!=' '&&s[i]!=','&&s[i]!=0){
				if(!myisalpha(s[i]))
					return ERROR;
				buffer[j++]=s[i++];
			}
			buffer[j]=0;
			table_res = which_table(O->table[0],O->table[1],buffer);
			if(table_res==ERROR||table_res==3)
				return ERROR;
			//simple_col_number[table_res]++;
			//strcpy(O->cols[table_res][simple_col_number[table_res]],buffer);
			O->which_group=table_res+1;
			strcpy(O->agg[table_res][0].col_name,buffer);
			while(s[i]==' ')
				i++;
			if(s[i]==','){
				i++;
				goto general;
			}
			if(s[i]==0)
				goto finish;
			return ERROR;
		}
		if(s[i]==','){
			i++;
			buffer[j]=0;
			strcpy(O->agg[2][0].col_name,buffer);//2-unknown col
			O->which_group=3;
			goto general;
		}
		return ERROR;

	}
	else{
		//simple col
		j=0;
		memset(buffer,0,MAX_TABLE_NAME_LEN);
		while(s[i]!=0&&s[i]!=',')
			buffer[j++]=s[i++];
		buffer[j]=0;
		table_res = which_table(O->table[0],O->table[1],buffer);
			if(table_res==ERROR||table_res==3)
				return ERROR;
			//simple_col_number[table_res]++;
			//strcpy(O->cols[table_res][simple_col_number[table_res]],buffer);
			simple_col_number[table_res]++;
			strcpy(O->cols[table_res][simple_col_number[table_res]],buffer);
			while(s[i]==' ')
				i++;
			if(s[i]==','){
				i++;
				goto general;
			}
			if(s[i]==0)
				goto finish;
			return ERROR;
	}
	finish:
	for(i=0;i<3;i++){
		O->num_cols[i]=simple_col_number[i];
		O->agg_number[i]=agg_col_number[i];
	}
	return OK;

	agg_check:
	if(op==2)
		i+=5;
	else
		i+=3;
	if(simple_col_number[0]+simple_col_number[1]+simple_col_number[2]
		>=2)
		return ERROR;
	else if(simple_col_number[0]+simple_col_number[1]+simple_col_number[2]==1){
		//need to put simple to group by col
		if(simple_col_number[0]){
			strcpy(O->agg[0][0].col_name,O->cols[0][1]);
			simple_col_number[0]=0;
			O->which_group=1;
		}
		else if(simple_col_number[1]){
			strcpy(O->agg[1][0].col_name,O->cols[1][1]);
			simple_col_number[1]=0;
			O->which_group=2;
		}
		else{
			strcpy(O->agg[2][0].col_name,O->cols[2][1]);
			simple_col_number[2]=0;
			O->which_group=3;
		}
		goto aggr;
	}
	else{
		goto aggr;
	}

	aggr:
	j=0;
	k=0;
	if(s[i]=='('){
		i++;
		while(s[i]==' ')
			i++;
	}
	else{
		while(s[i]==' ')
			i++;
		if(s[i]!='(')
			return ERROR;
		i++;
		while(s[i]==' ')
			i++;
	}
	//now we are at the start of the agg col
	while(s[i]!=' '&&s[i]!=')'){
		if(s[i]==0)
			return ERROR;
		buffer[j++]=s[i++];	
	}
	buffer[j] = 0;
	//check the column name:remove table name
	for(j=0;buffer[j]!=0;j++)
		if(buffer[j]=='.')
		{
			j++;
			k=0;
			while(buffer[j]!=0){
				buffer[k++]=buffer[j++];
			}
			buffer[k]=0;
		}
	if(s[i]==')'){
		i++;
		while(s[i]==' ')
			i++;
	}
	else{
		while(s[i]==' ')
			i++;
		if(s[i]!=')')
			return ERROR;
		i++;
		while(s[i]==' ')
			i++;
	}
	table_res = which_table(O->table[0],O->table[1],buffer);
	if(table_res==ERROR)
		return ERROR;
	if(table_res==3){
		if(op!=2)
			return ERROR;
		if(buffer[1]!=0);
			return ERROR;
		table_res=2;
	}
	agg_col_number[table_res]++;
	strcpy(O->agg[table_res][agg_col_number[table_res]].col_name,buffer);
	O->agg[table_res][agg_col_number[table_res]].op=op;
	if(s[i]==0)
		goto finish;
	if(s[i]!=',')
		return ERROR;
	i++;
	goto general;	

}
int extract_col1(arg_struct * O,char *s){
//having extracted the table name
	int col_count=0;
	int i=0;
	int j=0;
	int k=0;
	int agg_col_number = 0;
	int simple_col_number = 0;
	int group_col_number = 0;
	int fullname_flag = 0;
	if(!myisalpha2(s[0]))
		return ERROR;
	s1://read a new column
	if(group_col_number>1)
		return ERROR;
	while(s[i]==' ')
		i++;
	if(!myisalpha2(s[i]))
		return ERROR;
	//aggregation check
	if(s[i]=='s'&&s[i+1]=='u'&&s[i+2]=='m'&&(s[i+3]==' '||s[i+3]=='('))
		goto sum;	
	if(s[i]=='c'&&s[i+1]=='o'&&s[i+2]=='u'&&s[i+3]=='n'&&s[i+4]=='t'&&(s[i+5]==' '||s[i+5]=='('))
		goto count;	
	if(s[i]=='a'&&s[i+1]=='v'&&s[i+2]=='g'&&(s[i+3]==' '||s[i+3]=='('))	
		goto avg;
	if(s[i]=='m'&&s[i+1]=='i'&&s[i+2]=='n'&&(s[i+3]==' '||s[i+3]=='('))	
		goto min;
	if(s[i]=='m'&&s[i+1]=='a'&&s[i+2]=='x'&&(s[i+3]==' '||s[i+3]=='('))	
		goto max;
	//no aggregation, usual column
	if(agg_col_number>0&&group_col_number==0&&simple_col_number==0){
		//add a new group col
		goto s3;
	}
	else if(agg_col_number==0&&group_col_number==0){
		//add simple column
		goto s2;
	}
	else
		return ERROR;

	s3://get a group column
	group_col_number++;
	if(group_col_number!=1)
		return ERROR;
	fullname_flag = 0;
	j = 0;
	k = 0;
	while(s[i+k]!=','&&s[i+k]!='\0'){
		if(s[i+k]=='.'){
			k++;
			while(s[i+k]==' ')
				k++;
			if(s[i+k]==0)
				return ERROR;
			if(!myisalpha(s[i+k]))//need check first char
				return ERROR;
			fullname_flag = 1;
			i=i+k;
			while(s[i]!=','&&s[i]!=' '&&s[i]!=0){
				if(!myisalpha(s[i]))
					return ERROR;
				O->group_col[j++]=s[i++];
				}
			O->group_col[j]= 0;
			O->which_group=1;
			while(s[i]==' ')
				i++;
			if(s[i]==0)
					goto finish;
			if(s[i]!=',')
				return ERROR;
			i++;
			goto s1;
			}
		k++;
	}
	if(s[i+k]==0){
		while(s[i]!=0&&s[i]!=' ')
			O->group_col[j++]=s[i++];
		O->group_col[j] = 0;
		O->which_group=1;
		goto finish;
	}
	else{
		while(s[i]!=','&&s[i]!=' ')
			O->group_col[j++]=s[i++];
		O->group_col[j] = 0;
		O->which_group=1;
		while(s[i]==' ')
			i++;
		i++;
		goto s1;
	}
	return ERROR;	
	
	s2://get a simple column
		//sort if the col name contains table name(by .)
	
	simple_col_number++;
	fullname_flag = 0;
	j = 0;
	k = 0;
	while(s[i+k]!=','&&s[i+k]!='\0'){
		if(s[i+k]=='.'){
			k++;
			while(s[i+k]==' ')
				k++;
			if(s[i+k]==0)
				return ERROR;
			if(!myisalpha(s[i+k]))//need check first char
				return ERROR;
			fullname_flag = 1;
			i=i+k;
			while(s[i]!=','&&s[i]!=' '&&s[i]!=0){
				if(!myisalpha(s[i]))
				return ERROR;
				O->cols[0][simple_col_number][j++]=s[i++];
				}
			O->cols[0][simple_col_number][j] = 0;
			while(s[i]==' ')
				i++;
			if(s[i]==0)
					goto finish;
			if(s[i]!=',')
				return ERROR;
			i++;
			goto s1;
			}
		k++;
	}
	if(s[i+k]==0){
		while(s[i]!=0&&s[i]!=' ')
			O->cols[0][simple_col_number][j++]=s[i++];
		O->cols[0][simple_col_number][j] = 0;
		goto finish;
	}
	else{
		while(s[i]!=','&&s[i]!=' ')
			O->cols[0][simple_col_number][j++]=s[i++];
		O->cols[0][simple_col_number][j] = 0;
		while(s[i]==' ')
			i++;
		i++;
		goto s1;
	}
	return ERROR;

	sum:
	O->agg_number[0]++;
	O->agg[0][O->agg_number[0]].op=1;
	i+=3;
	goto get_agg_col;

	count:
	O->agg_number[0]++;
	O->agg[0][O->agg_number[0]].op=2;
	i+=5;
	goto get_agg_col;

	avg:
	O->agg_number[0]++;
	O->agg[0][O->agg_number[0]].op=3;
	i+=3;
	goto get_agg_col;

	min:
	O->agg_number[0]++;
	O->agg[0][O->agg_number[0]].op=4;
	i+=3;
	goto get_agg_col;

	max:
	O->agg_number[0]++;
	O->agg[0][O->agg_number[0]].op=5;
	i+=3;
	goto get_agg_col;
	finish:
	O->num_cols[0] = simple_col_number;
	O->agg_number[0] = agg_col_number;
	return OK;

	get_agg_col:
	//check if too many group cols
	if(simple_col_number>1)
		return ERROR;
	else if(simple_col_number==1&&group_col_number==1){
		return ERROR;
	}
	else if(simple_col_number==1&&group_col_number==0){
		//convert simple to group col
		simple_col_number=0;
		group_col_number=1;
		strcpy(O->group_col,O->cols[0][1]);
		O->which_group=1;
		memset(&O->cols[0][1],0,MAX_TABLE_NAME_LEN*sizeof(char));
	}
	j=0;
	k=0;
	if(s[i]=='('){
		i++;
		while(s[i]==' ')
			i++;
	}
	else{
		while(s[i]==' ')
			i++;
		if(s[i]!='(')
			return ERROR;
		i++;
		while(s[i]==' ')
			i++;
	}
	//now we are at the start of the agg col
	while(s[i]!=' '&&s[i]!=')'){
		if(s[i]==0)
			return ERROR;
		O->agg[0][O->agg_number[0]].col_name[j++]=s[i++];	
	}
	O->agg[0][O->agg_number[0]].col_name[j] = 0;
	//check the column name:remove table name
	for(j=0;O->agg[0][O->agg_number[0]].col_name[j]!=0;j++)
		if(O->agg[0][O->agg_number[0]].col_name[j]=='.')
		{
			j++;
			k=0;
			while(O->agg[0][O->agg_number[0]].col_name[j]!=0){
				O->agg[0][O->agg_number[0]].col_name[k++]=O->agg[0][O->agg_number[0]].col_name[j++];
			}
			O->agg[0][O->agg_number[0]].col_name[k]=0;
		}
	if(s[i]==')'){
		i++;
		while(s[i]==' ')
			i++;
	}
	else{
		while(s[i]==' ')
			i++;
		if(s[i]!=')')
			return ERROR;
		i++;
		while(s[i]==' ')
			i++;
	}
	agg_col_number++;
	if(s[i]==0)
		goto finish;
	if(s[i]!=',')
		return ERROR;
	i++;
	goto s1;

}

int how_many_tb(char *s,arg_struct *O){
//given table(s), split (them by ",")
	int count=0;
	int j=0;
	while(*(s+count)!=','&&*(s+count)!=0){
		if(myisalpha(*(s+count)))
			O->table[0][j++]=*(s+count);
		else if(*(s+count)==' '){
			count++;
			while(*(s+count)==' ')
				count++;
			break;
		}
		else
			return ERROR;
		count++;
	}
	O->table[0][j]=0;
	if(*(s+count)==0)
		return 1;
	if(*(s+count)!=',')
		return ERROR;
	count++;
	j=0;
	while(*(s+count)==' ')
		count++;
	while(*(s+count)!=0){
		if(myisalpha(*(s+count)))
			O->table[1][j++]=*(s+count);
		else if(*(s+count)==' '){
			count++;
			while(*(s+count)==' ')
				count++;
			break;
		}
		else
			return ERROR;
		count++;
	}
	if(*(s+count)!=0)
		return ERROR;
	O->table[1][j]=0;	
	return 2;
}
/*
int main(int argc, char const *argv[])
{
	char s[4][200] ={"select tab1.id, max(count) from tab1, tab2 where tab1.id = tab2.id and count <= 30 and tab1.id!=0 group by tab1.id;",
	"select id, count(*) from tab1 where id > 0 group by id;",
"select id, avg(name) from tab1 where id > 0 group by id;",
"select id, text from tab1 group by id;"};
	
	char middle_buffer[6][1000];
	char flags[6];
	int j;
	int i;
	int how_many;
	arg_struct argument;
	for(j=0;j<4;j++){
		printf("%s\n", s[j]);
		if(format_check(s[j],middle_buffer,flags)!=ERROR){	
			for (i = 0; i < 6; ++i)
			{
				printf("%d  ",flags[i] );
				printf("%s\n",middle_buffer[i] );
			}
			if((how_many=parse_select_begin(middle_buffer,flags,&argument))!=ERROR)
			{
				printf("type %d\n",how_many );
				printf("argument.table[0] = %s,argument.table[1] = %s\n", argument.table[0],argument.table[1]);
			}
			else{
				printf("syntax error\n");
			}
		}
		else
			printf("syntax error\n");
	}
	return 0;
}*/
int parse_select(arg_struct *O,char *s,char middle_buffer[6][1000],char sign_flag[6]){
		int how_many;
		if(format_check(s,middle_buffer,sign_flag)!=ERROR){	
			if((how_many=parse_select_begin(middle_buffer,sign_flag,O))!=ERROR)
			{
				return OK;
			}
			else{
				return ERROR;
			}
		}
		else
			return ERROR;	
}
int parse_select_begin(char middle_buffer[6][1000],char sign_flag[6],arg_struct *O){
	int table_number;
	int ex_col_result;
	int group_by_res;
	int i,j;
	memset(O,0,sizeof(arg_struct));
	table_number = how_many_tb(middle_buffer[1],O);
	O->table_number=table_number;
	if(table_number==ERROR)
		return ERROR;
	if(table_number==2){
		ex_col_result=extract_col2(O,middle_buffer[0]);
		if(ex_col_result==ERROR)
			return ERROR;
	}
	else{
		ex_col_result = extract_col1(O,middle_buffer[0]);
		if(ex_col_result==ERROR)
			return ERROR;
		if(O->which_group==1)//to satisfy argument API
			strcpy(O->agg[0][0].col_name,O->group_col);
	}
	if(sign_flag[5]==1){
		group_by_res=group_check(middle_buffer[5],O,table_number);
		if(group_by_res==ERROR)
			return ERROR;
	}
	int filter_res;
	if(sign_flag[2]){
		filter_res = filter(O,middle_buffer[2],table_number); 
		if(filter_res==ERROR)
			return ERROR;
	}	
	if(sign_flag[3]){
		filter_res = filter(O,middle_buffer[3],table_number); 
		if(filter_res==ERROR)
			return ERROR;
	}
	if(sign_flag[4]){
		filter_res = filter(O,middle_buffer[4],table_number); 
		if(filter_res==ERROR)
			return ERROR;
	}		
	printf("current info:\n");
	printf("tables:\n" );
	for(i=0;i<2;i++){
		if(table_number-1>=i)
			printf("table %d:%s\n",i+1,O->table[i] );
	}
	printf("output cols:\n");
	for(i=0;i<3;i++){
		printf("table %d:\n",i );
		for(j=0;j<O->num_cols[i];j++){
			printf("%s\t",O->cols[i][j+1]);
		}
		}
	printf("filtering constants:\n");
	printf("operations\n");
	printf("aggregations:\n");
		for(i=0;i<3;i++){
			printf("table %d aggs:\n", i);
			for(j=0;j<O->agg_number[i];j++){
				printf("op:%d\t",O->agg[i][j+1].op);
				printf("col_name:%s\n",O->agg[i][j+1].col_name );
			}
		}

	if(sign_flag[5]==1){
		printf("group by:\n");
		printf("%s\n",O->agg[O->which_group-1][0].col_name);
	}
	

				


	if(sign_flag[5]==0&&table_number==1)
		return 1;
	if(sign_flag[5]==0&&table_number==2)
		return 2;
	if(sign_flag[5]==1&&table_number==1)
		return 3;
	if(sign_flag[5]==1&&table_number==2)
		return 4;
	return ERROR;	
}
int check_scolon(char *s){
	int count=0;
	while(*(s+count)==' ')
		count++;
	if(*(s+count)==';')
		return 1;
	else if(*(s+count)==0)
		return ERROR;
	else
		return 0;
}
int check_and(char *c){
	int count  = 0;
	while(*(c+count)==' '){
		count++;
	}
	if(*(c+count)==0)
		return ERROR;
	if(*(c+count)==';')
		return 0;
	if(*(c+count) == 'a'&&*(c+count+1) == 'n'&&*(c+count+2) == 'd'&&*(c+count+3) == ' ')
		return count+4;
	return 1;
}
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
int myisalpha3(char *c){
	//match "from" to find the end of select ... from 
	int count = 0;
	if(*c==' '){
		while(*(c+count)==' ')
			count++;
		if(c+count == 0)
			return ERROR;
		if(*(c+count) == 'f'&&*(c+count+1) == 'r'&&*(c+count+2) == 'o'&&*(c+count+3) == 'm'&&*(c+count+4) == ' ')
			return count + 5;
	}
	return 1;
}
int myisalpha4(char *c){
	//match "where" and "group by" and ";"
	int count = 0;
	if(*c==0)
		return ERROR;
	if(*c==';')
		return 0;
	if(*c==' '){
		while(*(c+count)==' ')
			count++;
		if(*(c+count) == ';')
			return 0;
		if(*(c+count) == 0)
			return ERROR;
		if(*(c+count) == 'w'&&*(c+count+1) == 'h'&&*(c+count+2) == 'e'&&*(c+count+3) == 'r'&&*(c+count+4) == 'e'&&*(c+count+5) == ' ')
			return count + 6 + 1000;//identify where grom group by
		if(*(c+count) == 'g'&&*(c+count+1) == 'r'&&*(c+count+2) == 'o'&&*(c+count+3) == 'u'&&*(c+count+4) == 'p'&&*(c+count+5) == ' ')
		{
			count+=6;
			while(*(c+count)==' ')
				count++;
			if(*(c+count) == 'b'&&*(c+count+1) == 'y'&&*(c+count+2) == ' ')
				return count+3;
			return ERROR;//group appeared without by
		}
	}
	return 1;
}
int format_check(char *s,char middle_buffer[6][1000],char sign_flag[6]){
	unsigned char state = 0;
	short i = 0;
	short j = 0;
	for(j=0;j<6;j++){
		memset(middle_buffer[j],0,1000);
		sign_flag[j]=0;
	}
	int offset,offset2;
	//myisalpha2(*(s+i))
	while(1){
		switch(state){
			case 0:
			if(*(s+i)==' '){
				i++;
				break;
			}
			if(*(s+i)=='s'){
				i++;
				if(*(s+i)=='e'&&*(s+i+1)=='l'&&*(s+i+2)=='e'
					&&*(s+i+3)=='c'&&*(s+i+4)=='t'&&*(s+i+5)==' '){
					state = 1;
					i+=6;
					}
				else
					state = 200;//error
				break;
			}
			break;

			case 1:
			while(*(s+i)==' ')
				i++;
			if(myisalpha2(*(s+i))){
				for(j = 0;(offset=myisalpha3(s+i))==1;i++,j++){
					middle_buffer[0][j] = *(s+i);
					if(*(s+i) == 0)//if it does not contain from
						return ERROR;
				}
				if(offset==ERROR)
					return ERROR;
				i+=offset;
				middle_buffer[0][j] = 0;//add \0
				sign_flag[0] = 1;
				state = 2;
				break;
			}
			else
				return ERROR;
			break;

			case 2:
				while(*(s+i)==' ')
					i++;
				if(myisalpha2(*(s+i)))
				{
					//copy the characters to middle_buffer[1]
					//if group by go to 4
					//if where go to 3
					//if end go to 10
					//if meet unexpected \0 go to error
					for(j=0;(offset=myisalpha4(s+i))==1;i++,j++){
						middle_buffer[1][j]=*(s+i);
					}
					if(offset == 0){
						middle_buffer[1][j] = 0;
						sign_flag[1] = 1;
						state = 10;
						break;
					}
					if(offset>1000){
						offset-=1000;
						i+=offset;
						state = 3;
						middle_buffer[1][j] = 0;
						sign_flag[1] = 1;
						break;
					}
					if(offset==ERROR)
						return ERROR;
					middle_buffer[1][j] = 0;
					sign_flag[1] = 1;
					i+=offset;
					state = 4;
					break;
				}
				else
					return ERROR;
				break;

				case 3:
				while(*(s+i)==' ')
					i++;
				if(myisalpha2(*(s+i)))
				{
					//copy the characters to middle_buffer[2]
					//if group by go to 6
					//if and go to 5
					//if end go to 10
					//if meet unexpected \0 go to error
					for(j=0; ;i++,j++){
						offset = check_and(s+i);
						if(offset==ERROR)
							return ERROR;
						if(offset==0){
							middle_buffer[2][j] = 0;
							sign_flag[2] = 1;
							state = 10;
							break;
						}
						if(offset>1){
							i+=offset;
							middle_buffer[2][j] = 0;
							sign_flag[2] = 1;
							state = 5;
							break;
						}
						offset2 = myisalpha4(s+i);
						if(offset2==ERROR||offset2>1000||offset2==0)
							return ERROR;
						if(offset2>1){
							state = 6;
							i+=offset2;
							middle_buffer[2][j] = 0;
							sign_flag[2]=1;
							break;
						}
					middle_buffer[2][j]=*(s+i);
					}
					break;
				}
				else
					return ERROR;
				break;

				case 4:
				state = 8;
				break;
				case 5:
				while(*(s+i)==' ')
					i++;
				if(myisalpha2(*(s+i)))
				{
					//copy the characters to middle_buffer[3]
					//if group by go to 8
					//if and go to 7
					//if end go to 10
					//if meet unexpected \0 go to error
					for(j=0; ;i++,j++){
						offset = check_and(s+i);
						if(offset==ERROR)
							return ERROR;
						if(offset==0){
							middle_buffer[3][j] = 0;
							sign_flag[3]=1;
							state = 10;
							break;
						}
						if(offset>1){
							i+=offset;
							middle_buffer[3][j] = 0;
							sign_flag[3]=1;
							state = 7;
							break;
						}
						offset2 = myisalpha4(s+i);
						if(offset2==ERROR||offset2>1000||offset2==0)
							return ERROR;
						if(offset2>1){
							state = 8;
							i+=offset2;
							middle_buffer[3][j] = 0;
							sign_flag[3]=1;
							break;
						}
					middle_buffer[3][j]=*(s+i);
					}
					break;
				}
				else
					return ERROR;
				break;

				case 6:
				state = 8;
				break;

				case 7:
				while(*(s+i)==' ')
					i++;
				if(myisalpha2(*(s+i)))
				{
					//copy the characters to middle_buffer[4]
					//if group by go to 9
					//if end go to 10
					//if meet unexpected \0 go to error
					for(j=0; ;i++,j++){
						offset = myisalpha4(s+i);
						if(offset==ERROR||offset>1000)
							return ERROR;
						if(offset==0){
							middle_buffer[4][j] = 0;
							sign_flag[4]=1;
							state = 10;
							break;
						}
						if(offset>1){
							i+=offset;
							middle_buffer[4][j] = 0;
							sign_flag[4]=1;
							state = 9;
							break;
						}
						middle_buffer[4][j]=*(s+i);
					}
					break;
				}
				else
					return ERROR;
				break;
				case 8:
				//if(s[i]=='g'&&s[i+1]=='r'&&s[i+2]=='o'&&s[i+3]=='u'&&s[i+4]=='p'&&)
				//i+=8;
				while(*(s+i)==' ')
					i++;
				if(myisalpha2(*(s+i)))
				{
					//copy until semicolon
					//if end go to 10
					//otherwise go to error
					for(j=0; ;i++,j++){
						offset=check_scolon(s+i);
						if(offset==0)
							middle_buffer[5][j]=*(s+i);
						else if(offset==ERROR)
							return ERROR;
						else
							break;
					}
					middle_buffer[5][j]=0;
					sign_flag[5]=1;
					state = 10;
					break;
				}
				else
					return ERROR;
				break;

				case 9:
				state = 8;
				break;

				case 10:
				//OK now
				return OK;

				default:
				return ERROR;
		}
	}

}

