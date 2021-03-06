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
printf("num:%d\n",num);
    char name[128];
    int printbit[num+2];
    int no_group = (groupcol[0] == '\0');
    char agg_op[6][6] = {"","sum","count","avg","min","max"};
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
            if (strcmp(agg[j].col_name, head.col_name[i]) == 0 || (strcasecmp("*", agg[j].col_name)==0 && agg[j].op ==2)){
                printbit[j] = i;
                /**** 判断聚集列是不是int类型 ****/
                if (  agg[j].op != 2 && ((head.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                    printf("Column %s is not int and can’t be used in aggregation\n",agg[j].col_name);
                    fclose(fp);
                    return -1;
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
    if (!no_group) printf("%s",groupcol);
    if (num){
    char temp_buff[MAX_TABLE_NAME_LEN*num];
    for (i = 1; i <= num; i++) {
        sprintf(temp_buff, "|");
        sprintf(temp_buff+1, agg_op[agg[i].op]);
        sprintf(temp_buff+1+strlen(agg_op[agg[i].op]), "(%s)",agg[i].col_name);
    }
    printf("%s",temp_buff);
    
    }
    printf("\n");
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
            
            
            int flag = 0; //flag == 1 满足条件
            
            if (op) {
                if (constant.is_int) {//int
                    if (int_op(intarry[head.index[printbit[num + 1]]],constant.i,op))
                        flag = 1;
                } else {//varchar
                    char temp[128];
                    memcpy(temp, varchararry+varoffset[head.index[printbit[num + 1]]]-varoffset[0], varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]);
                    if(varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]])temp[varoffset[head.index[printbit[num + 1]]+1]-varoffset[head.index[printbit[num + 1]]]]='\0';
                    if (var_op(temp,constant.varchar,op))
                        flag = 1;
                }
            } else flag = 1;
            if (flag) {
                if (!no_group) {
                
                    find.is_int = type;
                    if (type)
                        find.i = intarry[head.index[printbit[0]]];
                    else {
                        memcpy(find.varchar,varchararry+varoffset[head.index[printbit[0]]]-varoffset[0],varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]);
                        //  printf("%d\n",varoffset[head.index[printbit[0]]+1]);
                         
                        if (varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]) find.varchar[varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]]='\0';
                    }
                    if ((group = lookup(find,agg,agg,num,0) )!= NULL){
                        group->a = find;
                        
                    }
                } else {
                    find.is_int = 0;
                    if (varoffset[head.index[printbit[0]]+1]-varoffset[head.index[printbit[0]]]) find.varchar[0] = '\0';
                    
                    if ((group = lookup(find,agg,agg,num,0) )!= NULL){
                        group->a = find;
                        
                    }
                }
                
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
            if (!no_group&&num!=0) {
                printf("|");
            }
            i = 1;
            if(num!=0){
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
            }
            printf("\n");
            q = q->next;
        }
    }
    
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
 int amb1
 int op1,                                                //选择条件1的操作（同上的op编码，op=0表示无此条件）
 int_or_char constant1,                                  //选择条件1的常量
 char *seletcol2,                                        //选择条件2的列
 int amb2,
 int op2,                                                //选择条件2的操作（同上的op编码，op=0表示无此条件）
 int_or_char constant2,                                  //选择条件2的常量
 char *seletcol3_1,                                      //连接条件的第一个列（来自table1）
 int amb3_1,
 int op3,                                                //连接条件的操作（同上的op编码，op=0表示无此条件）
 char *seletcol3_2                                       //连接条件的第而个列（来自table2）
 int amb3_2*/

int group_join(char *table1, char *groupcol1,aggregation *agg1, int num1, char *table2,char *groupcol2,aggregation *agg2, int num2, aggregation *unknown,int num, char *selectcol1, int amb1, int op1,  int_or_char constant1, char *selectcol2, int amb2, int op2, int_or_char constant2,  char *selectcol3_1, int amb3_1, int op3, char *selectcol3_2, int amb3_2){
    char name1[128];
    char name2[128];
    char agg_op[6][6] = {"","sum","count","avg","min","max"};
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
    int type1 = 0,type2 = 0;
    int grouptype1 = 0,grouptype2 = 0;
    int i,j = -1;
    int no_group = (groupcol1[0]=='\0'&&groupcol2[0]=='\0'&& unknown[0].col_name[0]=='\0');
    if (groupcol1[0]!='\0') {
        for (i = 0; i < head1.col_num; i++) {
            if (strcmp(groupcol1, head1.col_name[i]) == 0){
                printbit1[0] = i;
                if (  head1.col_type[i/32] & (1 << (i%32) )  ) {
                    
                    grouptype1 = 1;
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
    if (groupcol2[0]!='\0') {
        for (i = 0; i < head2.col_num; i++) {
            if (strcmp(groupcol2, head2.col_name[i]) == 0){
                printbit2[0] = i;
                if (  head2.col_type[i/32] & (1 << (i%32) )  ) {
                    
                    grouptype2 = 1;
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
                    if (strcmp(head2.col_name[j], unknown[t].col_name)==0) {
                        printf("Ambiguous column %s\n",unknown[t].col_name);
                        fclose(fp1);
                        fclose(fp2);
                        return  -1;
                    }
                }
                //未有歧义
                printbit1[0] = i;
                if (  head1.col_type[i/32] & (1 << (i%32) )  ) {
                    
                    grouptype1 = 1;
                }
                strcpy(groupcol1, unknown[t].col_name);
                break;
            }
        }
        //在table1中没有unknown[t]
        
        if (i >= head1.col_num) {
            
            for (j = 0; j < head2.col_num; j++) {
                
                if (strcmp(head2.col_name[j], unknown[t].col_name) == 0) {
                    printbit2[0] = j;
                    if (  head2.col_type[j/32] & (1 << (j%32) )  ) {
                        
                        grouptype2 = 1;
                    }
                    strcpy(groupcol2, unknown[t].col_name);
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
                if (strcmp(head1.col_name[i], unknown[t].col_name) == 0||unknown[t].op==2&&strcmp(unknown[t].col_name,"*")==0) {
               // printf("*******************************************%s\n",unknown[t].col_name);
                    for (j = 0; j < head2.col_num; j++) {
                        if (unknown[t].op!=2&&strcmp(head2.col_name[j], unknown[t].col_name)==0) {
                            printf("Ambiguous column %s\n",unknown[t].col_name);
                            fclose(fp1);
                            fclose(fp2);
                            return  -1;
                        }
                    }
                    //未有歧义
                    strcpy(agg1[num1+1].col_name, unknown[t].col_name);
                    printbit1[num1+1] = i;
                    agg1[num1+1].op = unknown[t].op;
                    if (  unknown[t].op != 2 && ((head1.col_type[i/32] & (1 << (i%32) ) ) == 0) ){
                        printf("Column %s is not int and can’t be used in aggregation",unknown[t].col_name);
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
                    
                    if (strcmp(head2.col_name[j], unknown[t].col_name) == 0) {
                    	printbit2[num2+1] = j;
                        strcpy(agg2[num2+1].col_name, unknown[t].col_name);
                        agg2[num2+1].op = unknown[t].op;
                        if (  unknown[t].op != 2 && ((head2.col_type[j/32] & (1 << (j%32) ) ) == 0) ){
                            printf("Column %s is not int and can’t be used in aggregation",unknown[t].col_name);
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
  //  int *printbit;
  //  aggregation *agg;
    /******* get group col **********/
    int varoffset1[head1.col_num - head1.intnum+1];
    int intarry1[head1.intnum];
    char varchararry1[(head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1];
    int varoffset2[head2.col_num - head2.intnum+1];
    int intarry2[head2.intnum];
    char varchararry2[(head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1];
  //  char groupcol[MAX_TABLE_NAME_LEN];
  //  int *varoffset;
  //  int *intarry;
  //  char *varchararry;
  //  table_head head;
   if (groupcol2[0] == 0) {
 //       strcpy(groupcol, groupcol1);
 //       groupcol[strlen(groupcol1)]='\0';
        type = grouptype1;
 //       printbit = printbit1;
 //       agg = agg1;
 //       varoffset = varoffset1;
 //       intarry = intarry1;
 //       varchararry = varchararry1;
  //      head = head1;
    } else {
    //    strcpy(groupcol, groupcol2);
    //    groupcol[strlen(groupcol2)]='\0';
        type = grouptype2;
   //     printbit = printbit2;
   //     agg = agg2;
    //    varoffset = varoffset2;
    //    intarry = intarry2;
     //   varchararry = varchararry2;
     //   head = head2;
    }
    
    
    int op_temp1=0;
    int op_temp2=0;
    int_or_char intemp1;
    int_or_char intemp2;
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
                    op_temp1 = op1;
                    intemp1 = constant1;
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
                    intemp1 = constant1;
                    op_temp1 = op1;
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
                        intemp2 = constant1;
                        op_temp2 = op1;
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
                    intemp2 = constant2;
                    op_temp2 = op2;
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
                    intemp2 = constant2;
                    op_temp2 = op2;
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
                            if (constant2.is_int) printf("Predicate %d error\n",constant2.i);
                            else printf("Predicate %s error\n",constant2.varchar);
                            fclose(fp2);
                            fclose(fp1);
                            return -1;
                        }
                        intemp1 = constant2;
                        op_temp1 = op2;
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
    op1 = op_temp1;
    op2 = op_temp2;
    constant1 = intemp1;
    constant2 = intemp2;
    
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
    
    for (i = 0; i < HASHSIZE ;i++){
        hashtab[i] = 0;
    }
    char temp[128];
    char temp1[128];
    char temp2[128];
    
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
                memcpy(varoffset2, p, (head2.col_num - head2.intnum+1)*sizeof(int));
                memcpy(intarry2, p+(head2.col_num - head2.intnum+1)*sizeof(int), head2.intnum*sizeof(int));
                int i;
                for (i = 0; i < (head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry2[i]=0;
                memcpy(varchararry2, p+(head2.col_num+1)*sizeof(int), (varoffset2[head2.col_num - head2.intnum]+1)*sizeof(char));
                
                /***** 从另一个table中读出一个数据 ***********/
                // char temp2[varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store]]]+1];
                
                int flag = 0; //flag == 1 满足条件
                
                if (op2) {
               
                    if (constant2.is_int) {//int
                        if (int_op(intarry2[head2.index[printbit2[num2_store + 1]]],constant2.i,op2)) flag += 1;
                    } else {//varchar
                        memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2_store + 1]]]-varoffset2[0], varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]);
                        if(varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]])temp2[varoffset2[head2.index[printbit2[num2_store + 1]]+1]-varoffset2[head2.index[printbit2[num2_store + 1]]]]='\0';
                        if (var_op(temp2,constant2.varchar,op2)) flag += 1;
                    }
                } else flag++;
                if (flag) {
                      //  printf("op:~~~~~~~~~~~~%d\n",intarry2[head2.index[printbit2[num2 + 1]]]);
                    flag = 0;
                    int other_pagenum = 0;
                    while (other_pagenum <= buffnum) {
                        char *other_p = table_buff+other_pagenum*PAGE_LEN;
                        
                        other_p += sizeof(int);
                        int other_index = sizeof(int);
                        //  printf("table_buff:%d\n",*(int *)(table_buff+other_pagenum*PAGE_LEN));
                        while (other_index < *(int *)(table_buff+other_pagenum*PAGE_LEN)) {
                            
                          //  int varoffset1[head1.col_num - head1.intnum+1];
                          //  int intarry1[head1.intnum];
                            memcpy(varoffset1, other_p, (head1.col_num - head1.intnum+1)*sizeof(int));
                            memcpy(intarry1, other_p+(head1.col_num - head1.intnum+1)*sizeof(int), head1.intnum*sizeof(int));
                          //  char varchararry1[(head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1];
                            for (i = 0; i < (head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry1[i]=0;
                            memcpy(varchararry1, other_p+(head1.col_num+1)*sizeof(int), (varoffset1[head1.col_num - head1.intnum]+1)*sizeof(char));
                        //    printf("op:~~~~~~~~~~~~%d\n",intarry1[head1.index[printbit1[num1 + 1]]]);
                            //printf("varchararry1:%s\n",varchararry1);
                            // char temp1[varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]+1];
                          //  printf("~~~~~~~%d~~~~~~~~~%d\n",intarry1[head1.index[printbit1[num1 + 1]]], intarry2[head2.index[printbit2[num2+1]]]);
                          
                            /****** 判断是否满足条件 *******/
                            if (type1) {
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],5)) flag += 1;
                            } else {
                                memcpy(temp1, varchararry1+varoffset1[head1.index[printbit1[num1 + 1]]]-varoffset2[0], varoffset1[head1.index[printbit1[num2 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]);
                                if(varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]) temp1[varoffset1[head1.index[printbit1[num1 + 1]]+1]-varoffset1[head1.index[printbit1[num1 + 1]]]]='\0';
                                // printf("temp1:%s %s\n",temp1,temp2);
                                if (var_op(temp1,temp2,7)) flag += 1;
                                // printf("flag: %d\n",flag);
                            }
                            
                            if (flag) {
                            
                                flag = 0;
                                if (!no_group) {
                                    find.is_int = type;
                                    if (type){
                                    
                                        if (groupcol2[0]==0)
                                            find.i = intarry1[head1.index[printbit1[0]]];
                                        else find.i = intarry2[head2.index[printbit2[0]]];
                                      
                                    }
                                    else {
                                        if (groupcol2[0]==0) {
                                        
                                            memcpy(find.varchar,varchararry1+varoffset1[head1.index[printbit1[0]]]-varoffset1[0],varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]);
                                            
                                            if(varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]])find.varchar[varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]]='\0';
                                        } else {
                                        
                                            memcpy(find.varchar,varchararry2+varoffset2[head2.index[printbit2[0]]]-varoffset2[0],varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]);
                                            
                                            if(varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]])find.varchar[varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]]='\0';
                                        }
                                        
                                    }
                              
                            
                                    if ((group = lookup(find,agg1,agg2,num1_store,num2_store) )!= NULL){
                                        group->a = find;
                                       
                                    }
                           
                                } else {
                                    find.is_int = 0;
                                    find.varchar[0] = '\0';
                                    
                                    if ((group = lookup(find,agg1,agg2,num1_store,num2_store) )!= NULL){
                                        group->a = find;
                                        
                                        
                                    }
                                    
                                }
                             
                                for (i = 1; i <= num1_store; i++) {
                                    if (agg1[1].op != 3) {
                                    
                                        group->res[i] = aggregation_op(group->res[i] , intarry1[head1.index[printbit1[i]]], agg1[i].op);
                                        
                                    } else {
                                        group->res[i] = aggregation_op(group->res[i] , intarry2[head2.index[printbit1[i]]], 1);
                                        group->res2[i] = aggregation_op(group->res2[i], intarry2[head2.index[printbit1[i]]], 2);
                                    }
                                }
                                
                                for (i = 1; i <= num2_store; i++) {
                                    if (agg2[i].op != 3){
                                        group->res1[i] = aggregation_op(group->res1[i] , intarry2[head2.index[printbit2[i]]], agg2[i].op);
                                    } else {
                                        group->res1[i] = aggregation_op(group->res1[i] , intarry2[head2.index[printbit2[i]]], 1);
                                        group->res12[i] = aggregation_op(group->res12[i], intarry2[head2.index[printbit2[i]]], 2);
                                    }
                                    
                                }
                                
                                
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
                memcpy(varoffset1, p, (head1.col_num - head1.intnum+1)*sizeof(int));
                memcpy(intarry1, p+(head1.col_num - head1.intnum+1)*sizeof(int), head1.intnum*sizeof(int));
                int i;
                for (i = 0; i < (head1.col_num - head1.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry1[i]=0;
                memcpy(varchararry1, p+(head1.col_num+1)*sizeof(int), (varoffset1[head1.col_num - head1.intnum]+1)*sizeof(char));
                
                /***** 从另一个table中读出一个数据 ***********/
                // char temp1[varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store]]]+1];
                memcpy(temp1, varchararry1+varoffset1[head1.index[printbit1[num1_store + 1]]]-varoffset1[0], varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store + 1]]]);
                if(varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store + 1]]])temp1[varoffset1[head1.index[printbit1[num1_store + 1]]+1]-varoffset1[head1.index[printbit1[num1_store + 1]]]]='\0';
                
                
                
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
                            
                            
                            memcpy(varoffset2, other_p, (head2.col_num - head2.intnum+1)*sizeof(int));
                            memcpy(intarry2, other_p+(head2.col_num - head2.intnum+1)*sizeof(int), head2.intnum*sizeof(int));
                            
                            for (i = 0; i < (head2.col_num - head2.intnum)*MAX_VARCHAR_LEN+1; i++) varchararry2[i]=0;
                            memcpy(varchararry2, other_p+(head2.col_num+1)*sizeof(int), (varoffset2[head2.col_num - head2.intnum]+1)*sizeof(char));
                            
                            //printf("varchararry2:%s\n",varchararry2);
                            // char temp2[varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]+1];
                            
                            /****** 判断是否满足条件 *******/
                            if (type2) {
                                if (int_op(intarry1[head1.index[printbit1[num1 + 1]]],intarry2[head2.index[printbit2[num2 + 1]]],5)) flag += 1;
                                
                            } else {
                                memcpy(temp2, varchararry2+varoffset2[head2.index[printbit2[num2 + 1]]]-varoffset1[0], varoffset2[head2.index[printbit2[num1 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]);
                                if(varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]])temp2[varoffset2[head2.index[printbit2[num2 + 1]]+1]-varoffset2[head2.index[printbit2[num2 + 1]]]]='\0';
                                // printf("temp2:%s %s\n",temp2,temp1);
                                if (var_op(temp1,temp2,7)) flag += 1;
                                
                                
                            }
                            if (flag) {
                             	flag=0;
                                if (!no_group) {
                                    find.is_int = type;
                                    if (type){
                                        if (groupcol2[0]==0)
                                            find.i = intarry1[head1.index[printbit1[0]]];
                                        else find.i = intarry2[head2.index[printbit2[0]]];
                                        
                                    }
                                    else {
                                        if (groupcol2[0]==0) {
                                        
                                            memcpy(find.varchar,varchararry1+varoffset1[head1.index[printbit1[0]]]-varoffset1[0],varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]);
                                            
                                            if(varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]])find.varchar[varoffset1[head1.index[printbit1[0]]+1]-varoffset1[head1.index[printbit1[0]]]]='\0';
                                        } else {
                                        
                                            memcpy(find.varchar,varchararry2+varoffset2[head2.index[printbit2[0]]]-varoffset2[0],varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]);
                                            
                                            if(varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]) find.varchar[varoffset2[head2.index[printbit2[0]]+1]-varoffset2[head2.index[printbit2[0]]]]='\0';
                                        }
                                        
                                    }
                                    
                                    if ((group = lookup(find,agg1,agg2,num1_store,num2_store) )!= NULL){
                                        group->a = find;

                                        
                                    }
                                } else {
                                    find.is_int = 0;
                                    find.varchar[0] = '\0';
                                    
                                    if ((group = lookup(find,agg1,agg2,num1_store,num2_store) )!= NULL){
                                        group->a = find;
                                         
                                        
                                    }
                                }
                                
                                for (i = 1; i <= num1_store; i++) {
                                    if (agg1[1].op != 3) {
                                        group->res[i] = aggregation_op(group->res[i] , intarry1[head1.index[printbit1[i]]], agg1[i].op);
                                    } else {
                                        group->res[i] = aggregation_op(group->res[i] , intarry2[head2.index[printbit1[i]]], 1);
                                        group->res2[i] = aggregation_op(group->res2[i], intarry2[head2.index[printbit1[i]]], 2);
                                    }
                                }
                                for (i = 1; i <= num2_store; i++) {
                                    if (agg2[i].op != 3){
                                        group->res1[i] = aggregation_op(group->res1[i] , intarry2[head2.index[printbit2[i]]], agg2[i].op);
                                    } else {
                                        group->res1[i] = aggregation_op(group->res1[i] , intarry2[head2.index[printbit2[i]]], 1);
                                        group->res12[i] = aggregation_op(group->res12[i], intarry2[head2.index[printbit2[i]]], 2);
                                    }
                                    
                                }
                               
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
    
    /******* print head ******/
    if (!no_group ){
        if (groupcol2[0]==0)
            printf("%s",groupcol1);
        else
            printf("%s",groupcol2);
    }
    if (num1_store+num2_store!=0){
    char temp_buff[MAX_TABLE_NAME_LEN*num];
    for (i = 1; i <= num1_store; i++) {
        sprintf(temp_buff, "|");
        sprintf(temp_buff+1, agg_op[agg1[i].op]);
        sprintf(temp_buff+1+strlen(agg_op[agg1[i].op]), "(%s)",agg1[i].col_name);
    }
    if (num1_store) printf("%s",temp_buff);
    for (i = 1; i <= num2_store; i++) {
        sprintf(temp_buff, "|");
        sprintf(temp_buff+1, agg_op[agg2[i].op]);
        sprintf(temp_buff+1+strlen(agg_op[agg2[i].op]), "(%s)",agg2[i].col_name);
    }
    if (num2_store) printf("%s",temp_buff);
    }
    printf("\n");
    /******* print data ******/
    for (j = 0; j < HASHSIZE; j++) {
        nlist *q = hashtab[j];
        while(q) {
            if (type) {
                printf("%d",(q->a).i);
            } else {
                printf("%s",(q->a).varchar);
            }
            if (!no_group&&num1_store+num2_store!=0) {
                printf("|");
            }
            i = 1;
            if(num1_store+num2_store!=0){
            if (num1_store) {
                if (agg1[i].op != 3) {
                    printf("%d",q->res[i]);
                } else {
                    printf("%d",q->res[i]/q->res2[i]);
                }
                
                for (i = 2; i <=num1_store; i++) {
                    if (agg1[i].op != 3) {
                        printf("|%d",q->res[i]);
                    } else {
                        printf("|%d",q->res[i]/q->res2[i]);
                    }
                }
            }
            if (num1_store && num2_store) {
                printf("|");
            }
            if (num2_store) {
                i = 1;
                if (agg2[i].op != 3) {
                    
                    printf("%d",q->res1[i]);
                } else {
                    printf("%d",q->res1[i]/q->res12[i]);
                }
                for (i = 2; i <=num2_store; i++) {
                    if (agg2[i].op != 3) {
                        printf("|%d",q->res1[i]);
                    } else {
                        printf("|%d",q->res1[i]/q->res12[i]);
                    }
                }
            }
            }
            printf("\n");
            q = q->next;
        }
    }
    fclose(fp1);
    fclose(fp2);
    return 0;
}



