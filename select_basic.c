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
char * read_between(char *s,char *left,char *right,char *t);
int myisalpha2(char c);
int myisalpha3(char *c);
int myisalpha4(char *c);
int check_and(char *c);
int check_scolon(char *s);
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
	for(j=0;j<4;j++){
		printf("%s\n", s[j]);
		if(format_check(s[j],middle_buffer,flags)!=ERROR){	
			for (i = 0; i < 6; ++i)
			{
				printf("%d  ",flags[i] );
				printf("%s\n",middle_buffer[i] );
			}
		}
		else
			printf("syntax error\n");
	}
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
/*
char * read_between(char *s,char *left,char *right,char *t){
	char * left_pos;
	char * right_pos;
	left_pos = strstr(s,left)+strlen(left);
	right_pos = strstr(left_pos,right);
	if(left_pos&&right_pos){
		int len = (int )(right_pos - left_pos);
		memcpy(t,left_pos,len);
		memset(t+len,'\0',1);
		return right_pos;
	}
	else if(left_pos){
		return 1;
	}
	else
		return NULL;
}*/
