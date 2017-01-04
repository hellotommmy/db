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
 aggregation *agg,      //聚集列（多个）的数组（从1开始，0浪费）
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
            fclose(fp);
            return -1;
        }
    }
    
    
    for (j = 1; j <= num; j++) {
        for (i = 0; i < head.col_num; i++) {
            if (strcmp(agg[j].col_name, head.col_name[i]) == 0){
                printbit[j] = i;
                /**** 判断聚集列是不是int类型 ****/
                if (  agg[j].op != 2 && ((head.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                    printf("Column %s is not int and can’t be used in aggregation",agg[j].col_name);
                    fclose(fp);
                    return -1;
                }
                break;
            }
        }
        if (i >= head.col_num){
            printf("Column %s doesn’t exist\n",agg[j].col_name);
            fclose(fp);
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
                    fclose(fp);
                    return -1;
                }
                break;
            }
        }
        if ((i >= head.col_num)){
            printf("Column %s doesn’t exist\n",selectcol);
            fclose(fp);
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
    fclose(fp);
    return 0;
}

/*int group_join(
               char *table1,                                           //table1 name
               char *groupcol1,                                        //table1分组列（空字符表示没有group by条件）
               aggregation *agg1,                                      //table1的分组聚集列（多个）的数组（聚集从1开始，分组列放0（如果没有分组，为空字符）
               int num1,                                               //table1的聚集列的个数（注意：不包含分组列数目，后面用的方便一点）
               char *table2,                                           //table2 name
               char *groupcol2,                                        //table2分组列（空字符表示没有group by条件）
               aggregation *agg2,                                      //table2的聚集列（多个）的数组（聚集从1开始，分组列放0（如果没有分组，为空字符）
               int num2,                                               //table2的聚集列的个数
               aggregation *unknown,                                   //未知的列名（同样0号方未知的分组列，如果没有0号就浪费）
               int num,                                                //未知的列的个数（num不包含分组列）
               char *seletcol1,                                        //选择条件1的列
               int op1,                                                //选择条件1的操作（同上的op编码，op=0表示无此条件）
               int_or_char constant1,                                  //选择条件1的常量
               char *seletcol2,                                        //选择条件2的列
               int op2,                                                //选择条件2的操作（同上的op编码，op=0表示无此条件）
               int_or_char constant2,                                  //选择条件2的常量
               char *seletcol3_1,                                      //连接条件的第一个列（来自table1）
               int op3,                                                //连接条件的操作（同上的op编码，op=0表示无此条件）
               char *seletcol3_2                                       //连接条件的第而个列（来自table2）*/

int group_join(char *table1, char *groupcol1,aggregation *agg1, int num1, char *table2,char *groupcol2,aggregation *agg2, int num2, aggregation *unknown,int num, char *selectcol1, int op1,  int_or_char constant1, char *selectcol2, int op2, int_or_char constant2,  char *selectcol3_1, int op3, char *selectcol3_2){
    char name1[128];
    char name2[128];
    char agg_op[6][5] = {"","sum","count","avg","min","max"};
    int num1_store,num2_store;
    int printbit1[num1+1+num];
    int printbit2[num2+1+num];
    sprintf(name1, "./db/%s.tbl",table1);
    sprintf(name2, "./db/%s.tbl",table2);
    if (access(name1, 0) == -1){
        printf("Table %s doesn’t exist\n",table1);
        return -1;
    }
    if (access(name2, 0) == -1){
        printf("Table %s doesn’t exist\n",table2);
        return -1;
    }
    FILE *fp1;
    FILE *fp2;
    fp1 = fopen(name1, "rb");
    fp2 = fopen(name2, "rb");
    table_head head1, head2;
    fread(&head1, sizeof(table_head), 1, fp1);
    fread(&head2, sizeof(table_head), 1, fp2);
    int type1,type2;
    int i,j = -1;
    int no_group = (groupcol1[0]=='\0'&&groupcol2[0]=='\0'&& unknown[0].col_name[0]=='\0');
    if (groupcol1[0]!='\0') {
        for (i = 0; i < head1.col_num; i++) {
            if (strcmp(selectcol1, head1.col_name[i]) == 0){
                printbit1[num1 + 1] = i;
                if (  (head1.col_type[i/32] & (1 << (i%32) ) ) ^ (constant1.is_int << i%32)){
                    if (constant1.is_int) printf("Predicate %d error\n",constant1.i);
                    else printf("Predicate %s error\n",constant1.varchar);
                    fclose(fp1);
                    fclose(fp1);
                    return -1;
                }
                break;
            }
        }
        if ((i >= head1.col_num)){
            printf("Column %s doesn’t exist\n",selectcol1);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }
    if (groupcol2[0]=='\0') {
        for (i = 0; i < head2.col_num; i++) {
            if (strcmp(selectcol2, head2.col_name[i]) == 0){
                printbit2[num2 + 1] = i;
                if (  (head2.col_type[i/32] & (1 << (i%32) ) ) ^ (constant2.is_int << i%32)){
                    if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                    else printf("Predicate %s error\n",constant2.varchar);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
                break;
            }
        }
        if ((i >= head2.col_num)){
            printf("Column %s doesn’t exist\n",selectcol2);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }
    /******** judge cols in table1 *********/
    if (num1) {
        for (j = 1; j <= num1; j++) {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(agg1[j].col_name, head1.col_name[i]) == 0){
                    printbit1[j] = i;
                    if (  agg1[j].op != 2 && ((head1.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                        printf("Column %s is not int and can’t be used in aggregation",agg1[j].col_name);
                        fclose(fp1);
                        fclose(fp2);
                        return -1;
                    }
                    break;
                }
            }
            if (i >= head1.col_num){
                printf("Column %s doesn’t exist\n",agg1[j].col_name);
                fclose(fp1);
                fclose(fp2);
                return -1;
            }
        }
    }
    /******** judge cols in table2 *********/
    if (num2) {
        for (j = 1; j <= num2; j++) {
            for (i = 0; i < head2.col_num; i++) {
                if (strcmp(agg2[j].col_name, head2.col_name[i]) == 0){
                    printbit2[j] = i;
                    if (  agg2[j].op != 2 && ((head2.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                        printf("Column %s is not int and can’t be used in aggregation",agg2[j].col_name);
                        fclose(fp1);
                        fclose(fp2);
                        return -1;
                    }
                    break;
                }
            }
            if (i >= head2.col_num){
                printf("Column %s doesn’t exist\n",agg2[j].col_name);
                fclose(fp1);
                fclose(fp2);
                return -1;
            }
        }
    }
    /****** judge unknown cols ******/
    int t = 0;
    if (unknown[0].col_name[0]!='\0') {
        for (i = 0; i < head1.col_num; i++) {
            if (strcmp(head1.col_name[i], unknown[t].col_name) == 0) {
                for (j = 0; j < head2.col_num; j++) {
                    if (strcmp(head2.col_name[j], unknown[t].col_name)) {
                        printf("Ambiguous column %s\n",unknown[t].col_name);
                        fclose(fp1);
                        fclose(fp2);
                        return  -1;
                    }
                }
                //未有歧义
                strcpy(groupcol1, unknown[t].col_name);
                num1++;
                break;
            }
        }
        //在table1中没有unknown[t]
        
        if (i >= head1.col_num) {
            
            for (j = 0; j < head2.col_num; j++) {
                
                if (strcmp(head2.col_name[i], unknown[t].col_name) == 0) {
                    strcpy(groupcol2, unknown[t].col_name);
                    num2++;
                    break;
                }
            }
            
            if (j >= head2.col_num) {
                printf("Column %s doesn’t exist\n",unknown[t].col_name);
                fclose(fp1);
                fclose(fp2);
                return -1;
            }
        }
    }
    
    if (num) {
        
        for (t = 1; t <= num; t++) {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(head1.col_name[i], unknown[t].col_name) == 0) {
                    for (j = 0; j < head2.col_num; j++) {
                        if (strcmp(head2.col_name[j], unknown[t].col_name)) {
                            printf("Ambiguous column %s\n",unknown[t].col_name);
                            fclose(fp1);
                            fclose(fp2);
                            return  -1;
                        }
                    }
                    //未有歧义
                    strcpy(agg1[num1+1].col_name, unknown[t].col_name);
                    agg1[num1+1].op = unknown[t].op;
                    if (  agg1[j].op != 2 && ((head1.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                        printf("Column %s is not int and can’t be used in aggregation",agg1[j].col_name);
                        fclose(fp1);
                        fclose(fp2);
                        return -1;
                    }
                    num1++;
                    break;
                }
            }
            //在table1中没有unknown[t]
            
            if (i >= head1.col_num) {
                
                for (j = 0; j < head2.col_num; j++) {
                    
                    if (strcmp(head2.col_name[i], unknown[t].col_name) == 0) {
                        strcpy(agg2[num2+1].col_name, unknown[t].col_name);
                        agg2[num2+1].op = unknown[t].op;
                        if (  agg2[j].op != 2 && ((head2.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                            printf("Column %s is not int and can’t be used in aggregation",agg2[j].col_name);
                            fclose(fp1);
                            fclose(fp2);
                            return -1;
                        }
                        num2++;
                        break;
                    }
                }
                
                if (j >= head2.col_num) {
                    printf("Column %s doesn’t exist\n",unknown[t].col_name);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
            }
        }
    }
    num1_store = num1;
    num2_store = num2;
    
    int type;
    /******* get group col **********/
    char *groupcol;
    if (groupcol2[0] == '\0') {
        strcpy(groupcol, groupcol1);
        type = type1;
    } else {
        strcpy(groupcol, groupcol2);
        type = type2;
    }
    /******* judge selectcol1 in table1 *********/
    if (op1 != 0) {
        for (i = 0; i < head1.col_num; i++) {
            if (strcmp(selectcol1, head1.col_name[i]) == 0){
                printbit1[num1 + 1] = i;
                if (  (head1.col_type[i/32] & (1 << (i%32) ) ) ^ (constant1.is_int << i%32)){
                    if (constant1.is_int) printf("Predicate %d error\n",constant1.i);
                    else printf("Predicate %s error\n",constant1.varchar);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
                num1++;
                break;
            }
        }
        if ((i >= head1.col_num)){
            printf("Column %s doesn’t exist\n",selectcol1);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }
    /******* judge selectcol2 in table2 *********/
    if (op2 != 0) {
        for (i = 0; i < head2.col_num; i++) {
            if (strcmp(selectcol2, head2.col_name[i]) == 0){
                printbit2[num2 + 1] = i;
                if (  (head2.col_type[i/32] & (1 << (i%32) ) ) ^ (constant2.is_int << i%32)){
                    if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                    else printf("Predicate %s error\n",constant2.varchar);
                    return -1;
                }
                num2++;
                break;
            }
        }
        if ((i >= head2.col_num)){
            printf("Column %s doesn’t exist\n",selectcol2);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }

    
    /******* judge selectcol3_1 in table1 *********/
    if (op3 != 0) {
        for (i = 0; i < head1.col_num; i++) {
            if (strcmp(selectcol3_1, head1.col_name[i]) == 0){
                if (  head1.col_type[i/32] & (1 << (i%32) ) )  type1 = 1;
                printbit1[num1 + 1] = i;
                //   num1++;
                break;
            }
        }
        if ((i >= head1.col_num)){
            printf("Column %s doesn’t exist\n",selectcol3_1);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }
    
    
    /******* judge selectcol3_2 in table2 *********/
    if (op3 != 0) {
        for (i = 0; i < head2.col_num; i++) {
            if (strcmp(selectcol3_2, head2.col_name[i]) == 0){
                if (  head2.col_type[i/32] & (1 << (i%32) ) )  type2 = 1;
                printbit2[num2 + 1] = i;
                //   num2++;
                break;
            }
        }
        if ((i >= head2.col_num)){
            printf("Column %s doesn’t exist\n",selectcol3_2);
            fclose(fp1);
            fclose(fp2);
            return -1;
        }
    }
    
    /******* 检查连接条件是否一致 ***********/
    if (type1 ^ type2){ //have some problem
        if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
        else printf("Predicate %s error\n",constant2.varchar);
        fclose(fp1);
        fclose(fp2);
        return -1;
    }
    
    /******* print head ******/
    
    
    /******* get size of each table **********/
    int size1,size2;
    size1 = file_size(name1);
    size2 = file_size(name2);
    /****** 根据选择条件x 从tablex中读出符合条件的值 *******/
    int buff_size = (size1 > size2)?size2:size1;
    char table_buff[buff_size];
    int_or_char find;
    nlist *group;
    
    /****** 从另一个table中每次读入一个page，进行选择和连接 *********/
    char buff[PAGE_LEN];
    if (size1 < size2){
        int buffnum;
        buffnum = writetobuff(table_buff, head1, fp1, constant1, printbit1, num1_store, op1);
        int pagenum = head2.datapage;
        zero(buff);
        while (pagenum <= head2.freepage) {
            fseek(fp2, pagenum*PAGE_LEN, SEEK_SET);
            zero(buff);
            fread(buff,sizeof(char), PAGE_LEN,fp2);
            int index = sizeof(int);
            char *p = buff;
            p += sizeof(int);
            while (index < *(int *)buff) {
                
                /****读出一个tuple的数据*****/
                int varoffset2[head2.col_num - head2.intnum+1];
                int intarry2[head2.intnum];
                memcpy(varoffset2, p, (head2.col_num - head2.intnum+1)*sizeof(int));
                memcpy(intarry2, p+(head2.col_num - head2.intnum+1)*sizeof(int), head2.intnum*sizeof(int));
                char varchararry2[(head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1];
                int i;
                for (i = 0; i < (head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry2[i]=0;
                memcpy(varchararry2, p+(head2.col_num+1)*sizeof(int), (varoffset2[head2.col_num - head2.intnum]+1)*sizeof(char));
                
                /***** 从另一个table中读出一个数据 ***********/
                char temp2[varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store]]]+1];
                memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2_store + 1]]]-varoffset2[0], varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]);
                temp2[varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]]='\0';
                
                if (!no_group) {
                    find.is_int = type;
                    if (type)
                        find.i = intarry2[head2.index[printbit2[0]]];
                    else {
                        memcpy(find.varchar,varchararry2+varoffset2[head2.index[printbit2[0]]]-varoffset2[0],varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]);
                        //  printf("%d\n",varoffset[head.index[printbit[0]]+1]);
                        //  printf("%d\n",varoffset[head.index[printbit[0]]]);
                        find.varchar[varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]]='\0';
                    }
                    if ((group = lookup(find,agg2,num2) )!= NULL){
                        group->a = find;
                        
                    }
                } else {
                    find.is_int = 0;
                    find.varchar[0] = '\0';
                    
                    if ((group = lookup(find,agg2,num2) )!= NULL){
                        group->a = find;
                        
                    }
                }
                
                int flag = 0; //flag == 1 满足条件
                if (op2) {
                    if (constant2.is_int) {//int
                        if (int_op(intarry2[head2.index[printbit2[num2_store + 1]]],constant2.i,op2)) flag += 1;
                    } else {//varchar
                        
                        if (var_op(temp2,constant2.varchar,op2)) flag += 1;
                    }
                } else flag++;
                
                if (flag) {
                    flag = 0;
                    int other_pagenum = 0;
                    while (other_pagenum <= buffnum) {
                        char *other_p = table_buff+other_pagenum*PAGE_LEN;
                        
                        other_p += sizeof(int);
                        int other_index = sizeof(int);
                        // printf("table_buff:%d\n",*(int *)(table_buff+other_pagenum*PAGE_LEN));
                        while (other_index < *(int *)(table_buff+other_pagenum*PAGE_LEN)) {
                            
                            int varoffset1[head1.col_num - head1.intnum+1];
                            int intarry1[head1.intnum];
                            memcpy(varoffset1, other_p, (head1.col_num - head1.intnum+1)*sizeof(int));
                            memcpy(intarry1, other_p+(head1.col_num - head1.intnum+1)*sizeof(int), head1.intnum*sizeof(int));
                            char varchararry1[(head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1];
                            for (i = 0; i < (head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry1[i]=0;
                            memcpy(varchararry1, other_p+(head1.col_num+1)*sizeof(int), (varoffset1[head1.col_num - head1.intnum]+1)*sizeof(char));
                            
                            //printf("varchararry1:%s\n",varchararry1);
                            char temp1[varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]+1];
                            
                            /****** 判断是否满足条件 *******/
                            if (type1) {
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],op3)) flag += 1;
                            } else {
                                memcpy(temp1, varchararry1+varoffset1[head1.index[printbit1[num1 + 1]]]-varoffset2[0], varoffset1[head1.index[printbit1[num2 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]);
                                temp1[varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]]='\0';
                                // printf("temp1:%s %s\n",temp1,temp2);
                                if (var_op(temp1,temp2,op3)) flag += 1;
                                // printf("flag: %d\n",flag);
                            }
                            if (flag) {
                                
                                
                                for (i = 1; i <= num2; i++) {
                                    if (agg2[i].op != 3){
                                        group->res[i] = aggregation_op(group->res[i] , intarry2[head2.index[printbit2[i]]], agg2[i].op);
                                    } else {
                                        group->res[i] = aggregation_op(group->res[i] , intarry2[head2.index[printbit2[i]]], 1);
                                        group->res2[i] = aggregation_op(group->res2[i], intarry2[head2.index[printbit2[i]]], 2);
                                    }
                                }
                                
                                flag = 0;
                            }
                            other_p += varoffset1[head1.col_num - head1.intnum];
                            other_index += varoffset1[head1.col_num - head1.intnum];
                        }
                        other_pagenum++;
                    }
                }
                p += varoffset2[head2.col_num - head2.intnum];
                index += varoffset2[head2.col_num - head2.intnum];
            }
            pagenum++;
        }
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
                if (agg2[i].op != 3) {
                    printf("%d",q->res[i]);
                } else {
                    printf("%d",q->res[i]/q->res2[i]);
                }
                for (i = 2; i <=num; i++) {
                    if (agg2[i].op != 3) {
                        printf("|%d",q->res[i]);
                    } else {
                        printf("|%d",q->res[i]/q->res2[i]);
                    }
                }
                printf("\n");
                q = q->next;
            }
        }
    }else {
        int buffnum;
        buffnum = writetobuff(table_buff, head2, fp2, constant2, printbit2, num2_store, op2);
        int pagenum = head1.datapage;
        zero(buff);
        while (pagenum <= head1.freepage) {
            fseek(fp1, pagenum*PAGE_LEN, SEEK_SET);
            zero(buff);
            fread(buff,sizeof(char), PAGE_LEN,fp1);
            int index = sizeof(int);
            char *p = buff;
            p += sizeof(int);
            while (index < *(int *)buff) {
                
                /****读出一个tuple的数据*****/
                int varoffset1[head1.col_num - head1.intnum+1];
                int intarry1[head1.intnum];
                memcpy(varoffset1, p, (head1.col_num - head1.intnum+1)*sizeof(int));
                memcpy(intarry1, p+(head1.col_num - head1.intnum+1)*sizeof(int), head1.intnum*sizeof(int));
                char varchararry1[(head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1];
                int i;
                for (i = 0; i < (head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry1[i]=0;
                memcpy(varchararry1, p+(head1.col_num+1)*sizeof(int), (varoffset1[head1.col_num - head1.intnum]+1)*sizeof(char));
                
                /***** 从另一个table中读出一个数据 ***********/
                char temp1[varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store]]]+1];
                memcpy(temp1, varchararry1+varoffset1[head1.index[printbit1[num1_store + 1]]]-varoffset1[0], varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store + 1]]]);
                temp1[varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store + 1]]]]='\0';
                
                if (!no_group) {
                    find.is_int = type;
                    if (type)
                        find.i = intarry1[head1.index[printbit1[0]]];
                    else {
                        memcpy(find.varchar,varchararry1+varoffset1[head1.index[printbit1[0]]]-varoffset1[0],varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]);
                        //  printf("%d\n",varoffset[head.index[printbit[0]]+1]);
                        //  printf("%d\n",varoffset[head.index[printbit[0]]]);
                        find.varchar[varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]]='\0';
                    }
                    if ((group = lookup(find,agg1,num1) )!= NULL){
                        group->a = find;
                        
                    }
                } else {
                    find.is_int = 0;
                    find.varchar[0] = '\0';
                    
                    if ((group = lookup(find,agg1,num1) )!= NULL){
                        group->a = find;
                        
                    }
                }
                
                int flag = 0; //flag == 1 满足条件
                if (op1) {
                    if (constant1.is_int) {//int
                        if (int_op(intarry1[head1.index[printbit1[num1_store + 1]]],constant1.i,op1)) flag += 1;
                    } else {//varchar
                        
                        if (var_op(temp1,constant1.varchar,op1)) flag += 1;
                    }
                } else flag++;
                
                if (flag) {
                    flag = 0;
                    int other_pagenum = 0;
                    while (other_pagenum <= buffnum) {
                        char *other_p = table_buff+other_pagenum*PAGE_LEN;
                        
                        other_p += sizeof(int);
                        int other_index = sizeof(int);
                        // printf("table_buff:%d\n",*(int *)(table_buff+other_pagenum*PAGE_LEN));
                        while (other_index < *(int *)(table_buff+other_pagenum*PAGE_LEN)) {
                            
                            int varoffset2[head2.col_num - head2.intnum+1];
                            int intarry2[head2.intnum];
                            memcpy(varoffset2, other_p, (head2.col_num - head2.intnum+1)*sizeof(int));
                            memcpy(intarry2, other_p+(head2.col_num - head2.intnum+1)*sizeof(int), head2.intnum*sizeof(int));
                            char varchararry2[(head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1];
                            for (i = 0; i < (head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry2[i]=0;
                            memcpy(varchararry2, other_p+(head2.col_num+1)*sizeof(int), (varoffset2[head2.col_num - head2.intnum]+1)*sizeof(char));
                            
                            //printf("varchararry2:%s\n",varchararry2);
                            char temp2[varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]+1];
                            
                            /****** 判断是否满足条件 *******/
                            if (type2) {
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],op3)) flag += 1;
                            } else {
                                memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2 + 1]]]-varoffset1[0], varoffset2[head2.index[printbit2[num1 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]);
                                temp2[varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]]='\0';
                                // printf("temp2:%s %s\n",temp2,temp1);
                                if (var_op(temp1,temp2,op3)) flag += 1;
                                // printf("flag: %d\n",flag);
                            }
                            if (flag) {
                                
                                
                                
                                
                                for (i = 1; i <= num1; i++) {
                                    if (agg1[i].op != 3){
                                        group->res[i] = aggregation_op(group->res[i] , intarry1[head1.index[printbit1[i]]], agg1[i].op);
                                    } else {
                                        group->res[i] = aggregation_op(group->res[i] , intarry1[head1.index[printbit1[i]]], 1);
                                        group->res2[i] = aggregation_op(group->res2[i], intarry1[head1.index[printbit1[i]]], 2);
                                    }
                                }
                                
                                
                                
                                
                                
                                
                                
                                
                                flag = 0;
                            }
                            other_p += varoffset2[head2.col_num - head2.intnum];
                            other_index += varoffset2[head2.col_num - head2.intnum];
                        }
                        other_pagenum++;
                    }
                }
                p += varoffset1[head1.col_num - head1.intnum];
                index += varoffset1[head1.col_num - head1.intnum];
            }
            pagenum++;
        }
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
                if (agg1[i].op != 3) {
                    printf("%d",q->res[i]);
                } else {
                    printf("%d",q->res[i]/q->res2[i]);
                }
                for (i = 2; i <=num; i++) {
                    if (agg1[i].op != 3) {
                        printf("|%d",q->res[i]);
                    } else {
                        printf("|%d",q->res[i]/q->res2[i]);
                    }
                }
                printf("\n");
                q = q->next;
            }
        }
    }
    fclose(fp1);
    fclose(fp2);
    return 0;
}
