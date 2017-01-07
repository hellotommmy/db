//
//  insert.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 1/2/17.
//  Copyright © 2017 冯凌璇. All rights reserved.
//
#include "comm.h"
int insert(char *table_name, int col, int_or_char *inchar){  //按行存储
    char name[128];
    sprintf(name, "./db/%s.tbl",table_name);
    if (access(name, 0) == -1){
        printf("Table %s doesn’t exist\n",table_name);
        return -1;
    }
    FILE *fp;
    fp = fopen(name, "r+");
 //   printf("____________%s\n",name );
    table_head head;
    
    fread(&head, sizeof(table_head), 1, fp);
    if (head.col_num != col) {
        printf("Wrong number of columns\n");  //列数不对
        return  -1;
    }
    if (!comptypebit(col, inchar,head)) {
        printf("Value and column type mismatch\n");   //列的类型不匹配
        return  -1;
    }
    //插入信息
    char buff[PAGE_LEN];
    zero(buff);
    fseek(fp, head.freepage*PAGE_LEN, SEEK_SET);
    fread(buff, sizeof(char), PAGE_LEN, fp);    //read the last page with free space into buffer
    int varoffset[col - head.intnum+1];
    int intarry[head.intnum];
    char varchararry[(col - head.intnum)*MAX_VARCHAR_LEN+1];
    int i,it = 0,vt = 1;
    for (i = 0; i < (col - head.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry[i]=0;
    int base = head.base;
    int j = -1;
    varoffset[0] = base;

    for (i = 0; i < col; i++) {
        if (i%32 == 0) j++;
        if ((head.col_type[j] & (1<<i)) != 0 ) { //int type
            intarry[it] = inchar[i].i;
            it++;
        } else {
            
            
            memcpy(varchararry+base-head.base, &(inchar[i].varchar),sizeof(char)*strlen(inchar[i].varchar));
            
            base += strlen(inchar[i].varchar);
            varoffset[vt] = base;
            vt++;
            
        }
    }
    
    if(col - head.intnum) {varchararry[varoffset[col - head.intnum]] = '\0';
    	varoffset[col - head.intnum] += 1;
	}
    if ((*(int *)buff + varoffset[col - head.intnum]) >= PAGE_LEN) {
        head.freepage += 1;
        fseek(fp, 0, SEEK_SET);
        fwrite(&head, sizeof(table_head), 1, fp);
        page_init(head.freepage,fp);
        zero(buff);
        fseek(fp, head.freepage*PAGE_LEN, SEEK_SET);
        fread(buff, sizeof(char), PAGE_LEN, fp);    //read the new page with free space into buffer
    }
    fseek(fp, head.freepage*PAGE_LEN, SEEK_SET);
    int buff_index =  *(int *)buff;
    *(int *)buff += varoffset[col - head.intnum];
    fwrite(buff, sizeof(int), 1, fp);   //write buffer
    fseek(fp, head.freepage*PAGE_LEN+buff_index, SEEK_SET);
    fwrite(varoffset, sizeof(int), col - head.intnum + 1, fp);
    fwrite(intarry, sizeof(int), head.intnum, fp);
    fwrite(varchararry, sizeof(char), strlen(varchararry)+1, fp);
    fclose(fp);
    return 0;
}

