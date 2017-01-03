//
//  drop.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 12/29/16.
//  Copyright © 2016 冯凌璇. All rights reserved.
//

#include "comm.h"
int drop(char *s){
    char name[128];
    sprintf(name, "./db/%s.tbl",s);
    if (remove(name) != -1){
        printf("Successfully dropped table %s!\n",s);
        return -1;
    }
    printf("Can’t drop table %s!\n",s);
    return 0;
}