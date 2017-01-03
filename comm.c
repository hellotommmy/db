//
//  comm.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 1/2/17.
//  Copyright © 2017 冯凌璇. All rights reserved.
//

#include "comm.h"
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