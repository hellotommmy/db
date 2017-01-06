//
//  select.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 1/2/17.
//  Copyright © 2017 冯凌璇. All rights reserved.
//

#include "comm.h"

int select_simple(char cols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN], int num, char *table, char *selectcol, int op, int_or_char constant){
    //列名s，列名数量，表名，需要选择的列 操作种类（编号按ppt），常量 （注：如果是select * 情况，则num=0，没有where的情况，op=0）
    //投影
    printf("number:%d\n",num);
    printf("operation:%d\n", op);
    char name[128];
    int printbit[num+2];
    sprintf(name, "./db/%s.tbl",table);
    if (access(name, 0) == -1){
        printf("Table %s doesn’t exist\n",table);
        return -1;
    }
    FILE *fp;
    fp = fopen(name, "rb");
    table_head head;
    
    fread(&head, sizeof(table_head), 1, fp);
    
    int i,j = -1;
    int star = (num == 0);
    /***** judge ****/
    if (star) {
        num = head.col_num;
        for (i = 1; i <= head.col_num; i++) {
            printbit[i] = i - 1;
        }
    } else {
        for (j = 1; j <= num; j++) {
            for (i = 0; i < head.col_num; i++) {
                if (strcmp(cols[j], head.col_name[i]) == 0){
                    printbit[j] = i;
                    break;
                }
            }
            if (i >= head.col_num){
                printf("Column %s doesn’t exist\n",cols[j]);
                fclose(fp);
                return -1;
            }
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
    
    /****** print ******/
    if (star) {
        printf("%s",head.col_name[0]);
        for (i = 1; i < head.col_num; i++) {
            printf("|%s",head.col_name[i]);
        }
        printf("\n");
    } else {
        printf("%s",cols[1]);
        for (j = 2; j <= num; j++) 
            printf("|%s",cols[j]);
        printf("\n");
    }
    int pagenum = head.datapage;
    char buff[PAGE_LEN];
    zero(buff);
    while (pagenum <= head.freepage) {
        fseek(fp, pagenum*PAGE_LEN, SEEK_SET);
        zero(buff);
        fread(buff,sizeof(char), PAGE_LEN,fp);
        readbuff(buff, head, printbit, num, constant, op);
        pagenum++;
    }
    fclose(fp);
    return 0;
}





/*int select_join(
 //char cols1[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],     //第一个表的列
 //int num1,                                               //第一个表的列的个数 （num1和num2同时为0 表示select*）
 //char *table1,                                           //第一个表名
 //char cols2[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],     //第二个表的列
 //int num2,                                               //第二个表的列的个数
 //char *table2,                                           //的二个表名
 //char unknowncols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],//未知的列
 //int num,                                                //未知的列的个数
 //char *seletcol1,                                        //选择条件1的列
 int amb1,
 //int op1,                                                //选择条件1的操作（同上的op编码，op=0表示无此条件）
 //int_or_char constant1,                                  //选择条件1的常量
 //char *seletcol2,                                        //选择条件2的列
 int amb2,
 //int op2,                                                //选择条件2的操作（同上的op编码，op=0表示无此条件）
 int_or_char constant2,                                  //选择条件2的常量
 //char *seletcol3_1,                                      //连接条件的第一个列（来自table1）
 //int amb3_1,
 //int op3,                                                //连接条件的操作（同上的op编码，op=0表示无此条件）
 //char *seletcol3_2,                                       //连接条件的第而个列（来自table2）
 int amb3_2
 )*/
 
int select_join(char cols1[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],
    int num1,char *table1,char cols2[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num2,char *table2,char unknowncols[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN],int num,char *selectcol1,int amb1,int op1, int_or_char constant1,char *selectcol2,int amb2,int op2, int_or_char constant2,char *selectcol3_1,int amb3_1,int op3, char *selectcol3_2,int amb3_2){
    printf("________________________________\n");
    printf("selectcol3_1:%s\n",selectcol3_1);
    printf("selectcol3_2:%s\n",selectcol3_2);
    printf("op1:%d,amb1 = %d, constant1 = %d\n", op1,amb1,constant1.i);
    printf("amb3:%d\n",amb3_1);
    char name1[128];
    char name2[128];
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
    int i,j = -1;
    int star = (num1 == 0 && num2 == 0);
    /******** judge cols in table1 *********/
    if (star) {
        num1 = num1_store = head1.col_num;
        for (i = 1; i <= head1.col_num; i++) {
            printbit1[i] = i - 1;
        }
        num2 = num2_store = head2.col_num;
        for (i = 1; i <= head2.col_num; i++) {
            printbit2[i] = i - 1;
        }
    } else {
        if (num1) {
            for (j = 1; j <= num1; j++) {
                for (i = 0; i < head1.col_num; i++) {
                    if (strcmp(cols1[j], head1.col_name[i]) == 0){
                        printbit1[j] = i;
                        break;
                    }
                }
                if (i >= head1.col_num){
                    printf("Column %s doesn’t exist\n",cols1[j]);
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
                    if (strcmp(cols2[j], head2.col_name[i]) == 0){
                        printbit2[j] = i;
                        break;
                    }
                }
                if (i >= head2.col_num){
                    printf("Column %s doesn’t exist\n",cols2[j]);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
            }
        }
    }
    
    
    /****** judge unknown cols ******/
    if (num) {
        
        int t;
        for (t = 1; t <= num; t++) {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(head1.col_name[i], unknowncols[t]) == 0) {
                    for (j = 0; j < head2.col_num; j++) {
                        if (strcmp(head2.col_name[j], unknowncols[t])==0) {
                            printf("Ambiguous column %s\n",unknowncols[t]);
                            fclose(fp1);
                            fclose(fp2);
                            return  -1;
                        }
                    }
                    //未有歧义
                    printbit1[num1+1] = i;
                    strcpy(cols1[num1+1], unknowncols[t]);
                    num1++;
                    break;
                }
            }
            //在table1中没有unknown[t]
            
            if (i >= head1.col_num) {
                
                for (j = 0; j < head2.col_num; j++) {
                    
                    if (strcmp(head2.col_name[j], unknowncols[t]) == 0) {
                        strcpy(cols2[num2+1], unknowncols[t]);
                        printbit2[num2+1] = j;
                        num2++;
                        break;
                    }
                }
                
                if (j >= head2.col_num) {
                    printf("Column %s doesn’t exist\n",unknowncols[t]);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
            }
        }
    }
    num1_store = num1;
    num2_store = num2;
    int type1=0,type2=0;
     /******* judge selectcol1 in table1 *********/
    if (op1 != 0) {
        if (!amb1) {
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
        } else {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(head1.col_name[i], selectcol1) == 0) {
                    for (j = 0; j < head2.col_num; j++) {
                        if (strcmp(head2.col_name[j], selectcol1)==0) {
                            printf("Ambiguous column %s\n",selectcol1);
                            fclose(fp1);
                            fclose(fp2);
                            return  -1;
                        }
                    }
                    //未有歧义
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
            //在table1中没有
            
            if (i >= head1.col_num) {
                
                for (j = 0; j < head2.col_num; j++) {
                    
                    if (strcmp(head2.col_name[j], selectcol1) == 0) {
                        printbit2[num2 + 1] = j;
                        if (  (head2.col_type[j/32] & (1 << (j%32) ) ) ^ (constant1.is_int << j%32)){
                            if (constant1.is_int) printf("Predicate %d error\n",constant1.i);
                            else printf("Predicate %s error\n",constant1.varchar);
                            fclose(fp1);
                            fclose(fp2);
                            return -1;
                        }
                        num2++;
                        break;
                    }
                }
                
                if (j >= head2.col_num) {
                    printf("Column %s doesn’t exist\n",selectcol1);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
            }
        }
        
    }
    /******* judge selectcol2 in table2 *********/
    if (op2 != 0) {
        if (!amb2) {
            for (i = 0; i < head2.col_num; i++) {
                if (strcmp(selectcol2, head2.col_name[i]) == 0){
                    printbit2[num2 + 1] = i;
                    if (  (head2.col_type[i/32] & (1 << (i%32) ) ) ^ (constant2.is_int << i%32)){
                        if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                        else printf("Predicate %s error\n",constant2.varchar);
                        fclose(fp2);
                        fclose(fp1);
                        return -1;
                    }
                    num2++;
                    break;
                }
            }
            if ((i >= head2.col_num)){
                printf("Column %s doesn’t exist\n",selectcol2);
                fclose(fp2);
                fclose(fp1);
                return -1;
            }
        } else {
            for (i = 0; i < head2.col_num; i++) {
                if (strcmp(head2.col_name[i], selectcol2) == 0) {
                    for (j = 0; j < head1.col_num; j++) {
                        if (strcmp(head1.col_name[j], selectcol2)==0) {
                            printf("Ambiguous column %s\n",selectcol2);
                            fclose(fp2);
                            fclose(fp1);
                            return  -1;
                        }
                    }
                    //未有歧义
                    printbit2[num2 + 1] = i;
                    if (  (head2.col_type[i/31] & (1 << (i%32) ) ) ^ (constant2.is_int << i%32)){
                        if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                        else printf("Predicate %s error\n",constant2.varchar);
                        fclose(fp2);
                        fclose(fp1);
                        return -1;
                    }
                    num2++;
                    break;
                }
            }
            //在table2中没有
            
            if (i >= head2.col_num) {
                
                for (j = 0; j < head1.col_num; j++) {
                    
                    if (strcmp(head1.col_name[j], selectcol2) == 0) {
                        printbit1[num1 + 1] = j;
                        if (  (head1.col_type[j/31] & (1 << (j%32) ) ) ^ (constant2.is_int << j%32)){
                 //           printf("********************************\n");
                            if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                            else printf("Predicate %s error\n",constant2.varchar);
                            fclose(fp2);
                            fclose(fp1);
                            return -1;
                        }
                        num1++;
                        break;
                    }
                }
                
                if (j >= head1.col_num) {
                    printf("Column %s doesn’t exist\n",selectcol2);
                    fclose(fp2);
                    fclose(fp1);
                    return -1;
                }
            }
        }
        
    }
    
    /******* judge selectcol3_1 in table1 *********/
    if (op3 != 0) {
        if (!amb3_1) {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(selectcol3_1, head1.col_name[i]) == 0){
                    if (  head1.col_type[i/32] & (1 << (i%32) ) )  type1 = 1;
                    printbit1[num1 + 1] = i;
                    if (amb3_1) {
                        for (j = 0; j < head2.col_num; j++) {
                            if (strcmp(head2.col_name[j], selectcol3_1) == 0) {
                                printf("Ambiguous column %s\n",selectcol3_1);
                                fclose(fp1);
                                fclose(fp2);
                                return  -1;
                            }
                        }
                    }
                    break;
                }
            }
            if ((i >= head1.col_num)){
                printf("Column %s doesn’t exist\n",selectcol3_1);
                fclose(fp1);
                fclose(fp2);
                return -1;
            }
        } else {
            for (i = 0; i < head1.col_num; i++) {
                if (strcmp(head1.col_name[i], selectcol3_1) == 0) {
                    if (  head1.col_type[i/32] & (1 << (i%32) ) )  type1 = 1;
                    for (j = 0; j < head2.col_num; j++) {
                        if (strcmp(head2.col_name[j], selectcol3_1)==0) {
                            printf("Ambiguous column %s\n",selectcol3_1);
                            fclose(fp1);
                            fclose(fp2);
                            return  -1;
                        }
                    }
                    //未有歧义
                    printbit1[num1 + 1] = i;
                    break;
                }
            }
            //在table1中没有
            
            if (i >= head1.col_num) {
                
                for (j = 0; j < head2.col_num; j++) {
                    
                    if (strcmp(head2.col_name[j], selectcol3_1) == 0) {
                        if (  head2.col_type[j/32] & (1 << (j%32) ) )  type2 = 1;
                        printbit2[num2 + 1] = j;
                        break;
                    }
                }
                
                if (j >= head2.col_num) {
                    printf("Column %s doesn’t exist\n",selectcol3_1);
                    fclose(fp1);
                    fclose(fp2);
                    return -1;
                }
            }
        }
    }
    
    
    /******* judge selectcol3_2 in table2 *********/
    if (op3 != 0) {
        if (!amb3_2) {
            for (i = 0; i < head2.col_num; i++) {
                if (strcmp(selectcol3_2, head2.col_name[i]) == 0){
                    if (  head2.col_type[i/32] & (1 << (i%32) ) )  type2 = 1;
                    printbit2[num2 + 1] = i;
                    if (amb3_2) {
                        for (j = 0; j < head1.col_num; j++) {
                            if (strcmp(head1.col_name[j], selectcol3_2) == 0) {
                                printf("Ambiguous column %s\n",selectcol3_2);
                                fclose(fp1);
                                fclose(fp2);
                                return  -1;
                            }
                        }
                    }
                    break;
                }
            }
            if ((i >= head2.col_num)){
                printf("Column %s doesn’t exist\n",selectcol3_2);
                fclose(fp1);
                fclose(fp2);
                return -1;
            }
        } else {
            for (i = 0; i < head2.col_num; i++) {
                if (strcmp(head2.col_name[i], selectcol3_2) == 0) {
                    if (  head2.col_type[i/32] & (1 << (i%32) ) )  type2 = 1;
                    for (j = 0; j < head1.col_num; j++) {
                        if (strcmp(head1.col_name[j], selectcol3_2)==0) {
                            printf("Ambiguous column %s\n",selectcol3_2);
                            fclose(fp2);
                            fclose(fp1);
                            return  -1;
                        }
                    }
                    //未有歧义
                    printbit2[num2 + 1] = i;
                    break;
                }
            }
            //在table2中没有
            
            if (i >= head2.col_num) {
                
                for (j = 0; j < head1.col_num; j++) {
                    
                    if (strcmp(head1.col_name[j], selectcol3_2) == 0) {
                        if (  head1.col_type[j/32] & (1 << (j%32) ) )  type1 = 1;
                        printbit1[num1 + 1] = j;
                        break;
                    }
                }
                
                if (j >= head1.col_num) {
                    printf("Column %s doesn’t exist\n",selectcol3_2);
                    fclose(fp2);
                    fclose(fp1);
                    return -1;
                }
            }
        }
    }
    
    /******* 检查连接条件是否一致 ***********/
    if (type1 ^ type2){ //have some problem
        printf("Join predicate error\n");
        fclose(fp1);
        fclose(fp2);
        return -1;
    }
    
    
    /******* print head ******/
    if (star) {
        printf("%s",head1.col_name[0]);
        for (i = 1; i < head1.col_num; i++) {
            printf("|%s",head1.col_name[i]);
        }
        printf("|");
        printf("%s",head2.col_name[0]);
        for (i = 1; i < head2.col_num; i++) {
            printf("|%s",head2.col_name[i]);
        }
        printf("\n");
    } else {
        if (num1_store)
            printf("%s",cols1[1]);
        for (j = 2; j <= num1_store; j++) printf("|%s",cols1[j]);
        if (num1_store && num2_store) printf("|");
        if (num2_store)
            printf("%s",cols2[1]);
        for (j = 2; j <= num2_store; j++) printf("|%s",cols2[j]);
        printf("\n");
    }
    
    
    /******* get size of each table **********/
    int size1,size2;
    size1 = file_size(name1);
    size2 = file_size(name2);
    /****** 根据选择条件x 从tablex中读出符合条件的值 *******/
    int buff_size = (size1 > size2)?size2:size1;
    char table_buff[buff_size];
    
    /****** 从另一个table中每次读入一个page，进行选择和连接 *********/
    char buff[PAGE_LEN];
    if (size1 <= size2){
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
                
                int flag = 0; //flag == 1 满足条件
                
                if (op2) {
                    if (constant2.is_int) {//int
                        if (int_op(intarry2[head2.index[printbit2[num2_store + 1]]],constant2.i,op2)) flag += 1;
                    } else {//varchar
                        memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2_store + 1]]]-varoffset2[0], varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]);
                        temp2[varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]]='\0';
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
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],5)) flag += 1;
                            } else {
                                memcpy(temp1, varchararry1+varoffset1[head1.index[printbit1[num1 + 1]]]-varoffset2[0], varoffset1[head1.index[printbit1[num2 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]);
                                temp1[varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]]='\0';
                                // printf("temp1:%s %s\n",temp1,temp2);
                                if (var_op(temp1,temp2,7)) flag += 1;
                                // printf("flag: %d\n",flag);
                            }
                            if (flag) {
                                
                                /****** print table1 *********/
                                i =1;
                                if (num1_store){
                                    if(head1.col_type[0] & (1 << printbit1[i])){// int type
                                        if (num1_store) printf("%d",intarry1[head1.index[printbit1[i]]]);
                                    } else {
                                        if (num1_store) {
                                            char temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]+1];
                                            memcpy(temp, varchararry1+varoffset1[head1.index[printbit1[i]]]-varoffset1[0], varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]);
                                            temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]]='\0';
                                            printf("%s",temp);
                                        }
                                    }
                                }
                                int j = 0;
                                for (i = 2; i <= num1_store; i++) {
                                    //    printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
                                    if ((i-1)%32 == 0) j++;
                                    if(head1.col_type[j] & (1 << printbit1[i])){// int type
                                        printf("|%d",intarry1[head1.index[printbit1[i]]]);
                                    } else {
                                        char temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]+1];
                                        memcpy(temp, varchararry1+varoffset1[head1.index[printbit1[i]]]-varoffset1[0], varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]);
                                        temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]]='\0';
                                        printf("|%s",temp);
                                    }
                                }
                                /****** print table2 *********/
                                if (num1_store && num2_store) printf("|");
                                i =1;
                                if(num2_store){
                                    if(head2.col_type[0] & (1 << printbit2[i])){// int type
                                        if (num2_store) printf("%d",intarry2[head2.index[printbit2[i]]]);
                                    } else {
                                        if (num2_store){
                                            char temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]+1];
                                            memcpy(temp, varchararry2+varoffset2[head2.index[printbit2[i]]]-varoffset2[0], varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]);
                                            temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]]='\0';
                                            printf("%s",temp);
                                        }
                                    }
                                }
                                j = 0;
                                for (i = 2; i <= num2_store; i++) {
                                    //    printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
                                    if ((i-1)%32 == 0) j++;
                                    if(head2.col_type[j] & (1 << printbit2[i])){// int type
                                        printf("|%d",intarry2[head2.index[printbit2[i]]]);
                                    } else {
                                        char temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]+1];
                                        memcpy(temp, varchararry2+varoffset2[head2.index[printbit2[i]]]-varoffset2[0], varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]);
                                        temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]]='\0';
                                        printf("|%s",temp);
                                    }
                                }
                                printf("\n");
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
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],5)) flag += 1;
                            } else {
                                memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2 + 1]]]-varoffset1[0], varoffset2[head2.index[printbit2[num1 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]);
                                temp2[varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]]='\0';
                                // printf("temp2:%s %s\n",temp2,temp1);
                                if (var_op(temp1,temp2,7)) flag += 1;
                                // printf("flag: %d\n",flag);
                            }
                            if (flag) {
                                
                                /****** print table1 *********/
                                i =1;
                                if (num1_store){
                                    if(head1.col_type[0] & (1 << printbit1[i])){// int type
                                        if (num1_store) printf("%d",intarry1[head1.index[printbit1[i]]]);
                                    } else {
                                        char temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]+1];
                                        memcpy(temp, varchararry1+varoffset1[head1.index[printbit1[i]]]-varoffset1[0], varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]);
                                        temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]]='\0';
                                        printf("%s",temp);
                                    }
                                }
                                int j = 0;
                                for (i = 2; i <= num1_store; i++) {
                                    //    printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
                                    if ((i-1)%32 == 0) j++;
                                    if(head1.col_type[j] & (1 << printbit1[i])){// int type
                                        printf("|%d",intarry1[head1.index[printbit1[i]]]);
                                    } else {
                                        char temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]+1];
                                        memcpy(temp, varchararry1+varoffset1[head1.index[printbit1[i]]]-varoffset1[0], varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]);
                                        temp[varoffset1[head1.index[printbit1[i]]+1]-varoffset1[head1.index[printbit1[i]]]]='\0';
                                        printf("|%s",temp);
                                    }
                                }
                                /****** print table2 *********/
                                if (num1_store && num2_store) printf("|");
                                i =1;
                                if (num2_store){
                                    if(head2.col_type[0] & (1 << printbit2[i])){// int type
                                        if (num2_store) printf("%d",intarry2[head2.index[printbit2[i]]]);
                                    } else {
                                        if (num2_store) {
                                            char temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]+1];
                                            memcpy(temp, varchararry2+varoffset2[head2.index[printbit2[i]]]-varoffset2[0], varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]);
                                            temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]]='\0';
                                            printf("%s",temp);
                                        }
                                    }
                                }
                                j = 0;
                                for (i = 2; i <= num2_store; i++) {
                                    //    printf("\n%d %d\n",printbit[i],head.index[printbit[i]]);
                                    if ((i-1)%32 == 0) j++;
                                    if(head2.col_type[j] & (1 << printbit2[i])){// int type
                                        printf("|%d",intarry2[head2.index[printbit2[i]]]);
                                    } else {
                                        char temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]+1];
                                        memcpy(temp, varchararry2+varoffset2[head2.index[printbit2[i]]]-varoffset2[0], varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]);
                                        temp[varoffset2[head2.index[printbit2[i]]+1]-varoffset2[head2.index[printbit2[i]]]]='\0';
                                        printf("|%s",temp);
                                    }
                                }
                                printf("\n");
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
    }
    fclose(fp1);
    fclose(fp2);
    printf("________________________________\n");
    return 0;
}



