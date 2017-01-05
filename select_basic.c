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
int parse_select_begin(char middle_buffer[6][1000],char sign_flag[6],arg_struct *O);
int which_table(char *table1,char *table2,char *query){
	//return 0 if table1, 1 if table 2,2 if unknown
	//meanwhile, cut table name from query
	//e.g table1=te1,table2=t2;query=t2 . id
	//return 1, and query=id after return
	//return ERROR if query format wrong, e.g t.a.d, t#d
	//3 if *
	if(query[0]=='*')
		return 3;
	int i=0;
	int first_space;
	int first_dot;
	if(!myisalpha2(s[0]))
		return ERROR;
	while(query[i]!='.'&&query[i]!=' ')
	{
		if(query[i]==0)
			return 2;
		if(!myisalpha(s[i]))
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
			while(query[i]!=' '||query[i]!=0)
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
			while(query[i]!=' '||query[i]!=0)
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
	char table_name_buffer[MAX_TABLE_NAME_LEN];
	int simple_col_number[3];
	if(!myisalpha2(s[0]))
		return ERROR;
	int op =0 ;
	general:
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
	agg_check:
	if(simple_col_number[0]+simple_col_number[1]+simple_col_number[2]>=2)
		return ERROR;
	else if(simple_col_number[0]+simple_col_number[1]+simple_col_number[2]==1){
		//need to put simple to
	}

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
		goto finish;
	}
	else{
		while(s[i]!=','&&s[i]!=' ')
			O->group_col[j++]=s[i++];
		O->group_col[j] = 0;
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
				O->cols[0][simple_col_number-1][j++]=s[i++];
				}
			O->cols[0][simple_col_number-1][j] = 0;
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
			O->cols[0][simple_col_number-1][j++]=s[i++];
		O->cols[0][simple_col_number-1][j] = 0;
		goto finish;
	}
	else{
		while(s[i]!=','&&s[i]!=' ')
			O->cols[0][simple_col_number-1][j++]=s[i++];
		O->cols[0][simple_col_number-1][j] = 0;
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
		strcpy(O->group_col,O->cols[0][0]);
		memset(&O->cols[0][0],0,MAX_TABLE_NAME_LEN*sizeof(char));
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
}
int parse_select_begin(char middle_buffer[6][1000],char sign_flag[6],arg_struct *O){
	int table_number;
	int ex_col_result;
	int i,j;
	memset(O,0,sizeof(arg_struct));
	table_number = how_many_tb(middle_buffer[1],O);
	if(table_number==ERROR)
		return ERROR;
	if(table_number==2){
		//extract_col2();
		printf("hello~~\n");
	}
	else{
		ex_col_result = extract_col1(O,middle_buffer[0]);
		if(ex_col_result==ERROR)
			return ERROR;
		else{
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
					printf("%s\t",O->cols[i][j]);
				}
			}
			printf("filtering constants:\n");

			printf("operations\n");

			printf("aggregations:\n");
			for(i=0;i<3;i++){
				printf("table %d aggs:\n", i);
				for(j=0;j<O->agg_number[i];j++){
					printf("op:%d\t",O->agg[i][j].op);
					printf("col_name:%s\n",O->agg[i][j+1].col_name );
				}
			}
			
			if(sign_flag[5]==1){
				printf("group by:\n");
				printf("%s\n",O->group_col);
			}

		}
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
							i+=offset;
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
							i+=offset;
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

