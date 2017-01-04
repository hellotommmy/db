//
//  comm.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 1/2/17.
//  Copyright © 2017 冯凌璇. All rights reserved.
//

#include "comm.h"

int file_size(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    int size = statbuf.st_size;
    
    return size;
}

void zero(char *buff) {
    int i;
    for ( i = 0; i < PAGE_LEN; i++)
        buff[i] = 0;
}

void settypebit(int col, int *col_type, table_head *head){
    head->intnum = 0;
    int i,j = -1;
    int it = 0,vt = 0;
    for (i = 0; i < BITNUM; i++) head->col_type[i] = 0;
    for (i = 0 ; i < col; i++) {
        if (i%32 == 0) j++;
        if (col_type[i%32] == 1) { //int --> bitmap = 1
            head->index[i] = it;
            head->intnum += 1;
            head->col_type[j] |= (1 << i);
            it ++;
        } else {
            head->index[i] = vt;
            vt++;
        }
        
    }
}

void page_init(int pagenum, FILE *fp){
    char buff[PAGE_LEN];
    *(int *) buff = sizeof(int);
    fseek(fp, pagenum*PAGE_LEN, SEEK_SET);
    fwrite(buff, sizeof(char), PAGE_LEN, fp);
}

bool_t comptypebit(int col, int_or_char *inchar,table_head head){
    int i,j=-1;
    for (i = 0; i < col; i++) {
        if (i%32 == 0) j++;
        if ((inchar[i].is_int << (i%32)) ^ (head.col_type[j] & (1 << (i%32)))) {
            return FALSE;
        }
    }
    return TRUE;
}

int int_op(int a, int b, int op){
    switch (op) {
        case 1:
            if (a < b) return 1;
            break;
        case 2:
            if (a <= b) return 1;
            break;
        case 3:
            if (a > b) return 1;
            break;
        case 4:
            if (a >= b) return 1;
            break;
        case 5:
            if (a == b) return 1;
            break;
        case 6:
            if (a != b) return 1;
            break;
        default:
            break;
    }
    return 0;
}

int var_op(char *a, char *b, int op){
    switch (op) {
        case 7:
            if (strcmp(a, b) == 0) return  1;
            break;
        case 8:
            if (strcmp(a, b) != 0) return  1;
            break;
        case 9:
            if (strstr(a, b)) return  1;
            break;
        case 10:
            if (strstr(a, b) == NULL) return  1;
            break;
        default:
            break;
    }
    return 0;
}

void readbuff(char *buff, table_head head, int *printbit, int num, int_or_char constant, int op){
    int index = sizeof(int);
    char *p = buff;
    p += sizeof(int);
    while (index < *(int *)buff) {
        
        int varoffset[head.col_num - head.intnum+1];
        int intarry[head.intnum];
        /****读出一个tuple的数据*****/
        memcpy(varoffset, p, (head.col_num - head.intnum+1)*sizeof(int));
        memcpy(intarry, p+(head.col_num - head.intnum+1)*sizeof(int), head.intnum*sizeof(int));
        char varchararry[(head.col_num - head.intnum)*MAX_VARCHAR_LEN+1];
        int i;
        for (i = 0; i < (head.col_num - head.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry[i]=0;
        memcpy(varchararry, p+(head.col_num+1)*sizeof(int), (varoffset[head.col_num - head.intnum]+1)*sizeof(char));
        int flag = 0; //flag == 1 满足条件
        if (op) {
            if (constant.is_int) {//int
                if (int_op(intarry[head.index[printbit[num + 1]]],constant.i,op)) flag = 1;
            } else {//varchar
                char temp[varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]+1];
                memcpy(temp, varchararry+varoffset[head.index[printbit[num + 1]]]-varoffset[0], varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]);
                temp[varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]]='\0';
                if (var_op(temp,constant.varchar,op)) flag = 1;
            }
        }
        if (flag || op == 0) {
            i =1;
            //   printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
            if(head.col_type[0] & (1 << printbit[i])){// int type
                printf("%d",intarry[head.index[printbit[i]]]);
            } else {
                char temp[varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]+1];
                memcpy(temp, varchararry+varoffset[head.index[printbit[i]]]-varoffset[0], varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]);
                temp[varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]]='\0';
                printf("%s",temp);
            }
            int j = 0;
            for (i = 2; i <= num; i++) {
                //    printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
                if ((i-1)%32 == 0) j++;
                if(head.col_type[j] & (1 << printbit[i])){// int type
                    printf("|%d",intarry[head.index[printbit[i]]]);
                } else {
                    char temp[varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]+1];
                    memcpy(temp, varchararry+varoffset[head.index[printbit[i]]]-varoffset[0], varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]);
                    temp[varoffset[head.index[printbit[i]]+1]-varoffset[head.index[printbit[i]]]]='\0';
                    printf("|%s",temp);
                }
            }
            printf("\n");
        }
        p += varoffset[head.col_num - head.intnum];
        index += varoffset[head.col_num - head.intnum];
    }
}

void buff_init(char *buff){
    zero(buff);
    *(int *)buff = sizeof(int);
}

int buff_write(char *buff, int *varoffset, int *intarry, char *varchararry, int col_num, int intnum){
    if ((*(int *)buff + varoffset[col_num - intnum]) >= PAGE_LEN) {
        return -1;
    }
    memcpy(buff + *(int *)buff, varoffset, sizeof(int)*(col_num - intnum + 1));
    memcpy(buff + *(int *)buff + sizeof(int)*(col_num - intnum + 1), intarry, sizeof(int)*intnum);
    memcpy(buff + *(int *)buff + sizeof(int)*(col_num + 1), varchararry, strlen(varchararry)+1);
    *(int *)buff += varoffset[col_num - intnum];
    return 0;
}