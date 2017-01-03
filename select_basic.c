/*int select(char cols[][129], int num, 
char *table, 
char *seletcol, 
int op, 
int_or_char constant)
//列名s，
列名数量，
表名，
需要选择的列 操作种类（编号按ppt），
常量 （注：如果是select * 情况，则num=0，没有where的情况，op=0）
*/
#include <string.h>
#include <stdio.h>
#include "comm.h"
int format_check(char *s,char middle_buffer[6][1000]);
int read_between(char *s,char *left,char *right,char *t);

int main(int argc, char const *argv[])
{
	char s[200] = "select tab1.id, max(count) from tab1, tab2 where tab1.id = tab2.id and count <= 30 and tab1.id!=0 group by tab1.id;";
	char middle_buffer[6][1000];
	format_check(s,middle_buffer);
	int i;
	for (i = 0; i < 6; ++i)
	{
		printf("%s\n",middle_buffer[i] );
	}
	return 0;
}
int format_check(char *s,char middle_buffer[6][1000]){
	int aggre,join,filter1,filter2;
	int result[6];
	result[0] = read_between(s,"select","from",middle_buffer[0]);
	result[1] = read_between(s,"from","where",middle_buffer[1]);
	result[2] = read_between(s,"where","and",middle_buffer[2]);
	result[3] = read_between(s,"and","and",middle_buffer[3]);
	result[4] = read_between(s,"and","group by",middle_buffer[4]);
	result[5] = read_between(s,"group by",";",middle_buffer[2]);

}

int read_between(char *s,char *left,char *right,char *t){
	char * left_pos;
	char * right_pos;
	left_pos = strstr(s,left)+strlen(left);
	right_pos = strstr(s,right);
	if(left_pos&&right_pos){
		int len = (int )(right_pos - left_pos);
		memcpy(t,left_pos,len);
		memset(t+len,'\0',1);
		return OK;
	}
	else if(left_pos){
		return RIGHT_NOT_EXIST;
	}
	else
		return ERROR;
}
