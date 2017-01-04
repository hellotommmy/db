//
//  group.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 1/4/17.
//  Copyright © 2017 冯凌璇. All rights reserved.
//

#include "comm.h"

/*int group_simple(
 char *table,           //table name
 char *groupcol,        //分组列
 congregate *cong,      //聚集列（多个）的数组（从1开始，0浪费）
 int num,               //聚集列的个数
 char *seletcol,        //选择条件的列名
 int op,                //选择条件操作数（0表示没有where选择条件 其他同select）
 int_or_char constant   //选择条件常量
 )*/


int group_simple(char *table, char *groupcol, aggregation *agg, int num, char *selectcol, int op, int_or_char constant){
    char name[128];
    int printbit[num+2];
    int no_group = (groupcol == '\0');
    char agg_op[6][5] = {"","sum","count","avg","min","max"};
    sprintf(name, "./db/%s.tbl",table);
    if (access(name, 0) == -1){
        
        printf("Table %s doesn’t exist",table);
        return -1;
    }
    FILE *fp;
    fp = fopen(name, "rb");
    table_head head;
    
    fread(&head, sizeof(table_head), 1, fp);
    int i,j = -1,type = 0;
    
    if (!no_group) {
        for (i = 0; i < head.col_num; i++) {
            if (strcmp(groupcol, head.col_name[i]) == 0){
                printbit[0] = i;
                if (  head.col_type[i/32] & (1 << (i%32) )  ) {
                    type = 1;
                }
                break;
            }
        }
        if (i >= head.col_num){
            printf("Column %s doesn’t exist\n",groupcol);
            return -1;
        }
    }
        
        
    for (j = 1; j <= num; j++) {
        for (i = 0; i < head.col_num; i++) {
            if (strcmp(agg[j].col_name, head.col_name[i]) == 0){
                printbit[j] = i;
                /**** 判断聚集列是不是int类型 ****/
                if (  op != 2 && ((head.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                    printf("Column %s is not int and can’t be used in aggregation",agg[j].col_name);
                }
                break;
            }
        }
        if (i >= head.col_num){
            printf("Column %s doesn’t exist\n",agg[j].col_name);
            return -1;
        }
    }
    
    if (op != 0) {
        for (i = 0; i < head.col_num; i++) {
            if (strcmp(selectcol, head.col_name[i]) == 0){
                printbit[num + 1] = i;
                if (  (head.col_type[i/32] & (1 << (i%32) ) ) ^ (constant.is_int << i%32)){
                    if (constant.is_int) printf("Predicate %d error\n",constant.i);
                    else printf("Predicate %s error\n",constant.varchar);
                    return -1;
                }
                break;
            }
        }
        if ((i >= head.col_num)){
            printf("Column %s doesn’t exist\n",selectcol);
            return -1;
        }
    }
    
    /****** print title ******/
    printf("%s",groupcol);
    char temp_buff[MAX_TABLE_NAME_LEN*num];
    for (i = 1; i <= num; i++) {
        sprintf(temp_buff, "|");
        sprintf(temp_buff+1, agg_op[agg[i].op]);
        sprintf(temp_buff+1+strlen(agg_op[agg[i].op]), "(%s)",agg[i].col_name);
    }
    printf("%s\n",temp_buff);
    
    
    
    /****** group by ********/
    
    /********* empty hash table ***********/
    for (i = 0; i < HASHSIZE ;i++){
        hashtab[i] = 0;
    }
    int_or_char find;
    nlist *group;
    int pagenum = head.datapage;
    char buff[PAGE_LEN];
    zero(buff);
    while (pagenum <= head.freepage) {
        fseek(fp, pagenum*PAGE_LEN, SEEK_SET);
        zero(buff);
        fread(buff,sizeof(char), PAGE_LEN,fp);
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
            
            if (!no_group) {
                find.is_int = type;
                if (type)
                    find.i = intarry[head.index[printbit[0]]];
                else {
                    memcpy(find.varchar,varchararry+varoffset[head.index[printbit[0]]]-varoffset[0],varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]);
                    //  printf("%d\n",varoffset[head.index[printbit[0]]+1]);
                    //  printf("%d\n",varoffset[head.index[printbit[0]]]);
                    find.varchar[varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]]='\0';
                }
                if ((group = lookup(find,agg,num) )!= NULL){
                    group->a = find;

                }
            } else {
                find.is_int = 0;
                    find.varchar[0] = '\0';
                
                if ((group = lookup(find,agg,num) )!= NULL){
                    group->a = find;

                }
            }
            int flag = 0; //flag == 1 满足条件
            
            if (op) {
                if (constant.is_int) {//int
                    if (int_op(intarry[head.index[printbit[num + 1]]],constant.i,op))
                        flag = 1;
                } else {//varchar
                    char temp[varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]+1];
                    memcpy(temp, varchararry+varoffset[head.index[printbit[num + 1]]]-varoffset[0], varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]);
                    temp[varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]]='\0';
                    if (var_op(temp,constant.varchar,op))
                        flag = 1;
                }
            } else flag = 1;
            if (flag) {
                
                
                for (i = 1; i <= num; i++) {
                    if (agg[i].op != 3){
                        group->res[i] = aggregation_op(group->res[i] , intarry[head.index[printbit[i]]], agg[i].op);
                    } else {
                        group->res[i] = aggregation_op(group->res[i] , intarry[head.index[printbit[i]]], 1);
                        group->res2[i] = aggregation_op(group->res2[i], intarry[head.index[printbit[i]]], 2);
                    }
                }
            }
            
            p += varoffset[head.col_num - head.intnum];
            index += varoffset[head.col_num - head.intnum];
        }
        pagenum++;
    }
    /****** print **********/
    
    for (j = 0; j < HASHSIZE; j++) {
        nlist *q = hashtab[j];
        while(q) {
            if (type) {
                printf("%d",(q->a).i);
            } else {
                printf("%s",(q->a).varchar);
            }
            if (!no_group) {
                printf("|");
            }
            i = 1;
            if (agg[i].op != 3) {
                printf("%d",q->res[i]);
            } else {
                printf("%d",q->res[i]/q->res2[i]);
            }
            for (i = 2; i <=num; i++) {
                if (agg[i].op != 3) {
                    printf("|%d",q->res[i]);
                } else {
                    printf("|%d",q->res[i]/q->res2[i]);
                }
            }
            printf("\n");
            q = q->next;
        }
    }
    
    return 0;
}