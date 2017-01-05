//
//  create.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 12/22/16.
//  Copyright © 2016 冯凌璇. All rights reserved.
//
#include "comm.h"
int create(char *table_name, int col, char col_name[MAX_ITEMS_IN_TABLE][MAX_TABLE_NAME_LEN], int *col_type){
    char name[128];
    sprintf(name, "./db/%s.tbl",table_name);
    if (access(name, 0) == 0){
        printf("Can’t create table %s!\n",table_name);
        return -1;
    }
    table_head head;
    head.col_num = col;
    settypebit(col, col_type, &head);
    head.base = (col+1)*sizeof(int);
    head.datapage = (col<=ONE_PAGE_ITEM)?1:(col<=TWO_PAGE_ITEM)?2:(col<=THREE_PAGE_ITEM)?3:4;
    head.freepage = head.datapage;
    int i;
    for (i = 0; i < col; i++) {
        strcpy(head.col_name[i], col_name[i]);
    }
    FILE *fp;
    fp = fopen(name, "wb");
    int size = head.datapage*PAGE_LEN;
    if (fwrite(&head, sizeof(size), 1, fp) != 1) return -1;
    page_init(head.freepage,fp);
    printf("Successfully created table %s!\n",table_name);
    fclose(fp);
    return 0;
}
