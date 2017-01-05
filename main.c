//
//  main.c
//  dbms-lab3
//
//  Created by 冯凌璇 on 12/22/16.
//  Copyright © 2016 冯凌璇. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define ONE_PAGE_ITEM 31
#define TWO_PAGE_ITEM 62
#define THREE_PAGE_ITEM 94

#define PAGE_LEN 4096
#include "comm.h"






int main(int argc, const char * argv[]) {
    // insert code here...
    char s1[2][128] = {"id","name"};
    char s2[3][128]={"id","count","text"};

    int t1[2]={1,2};
    int t2[3]={1,1,2};
    
    int_or_char inch11[2];
    int_or_char inch12[2];
    int_or_char inch13[2];
    int_or_char inch14[2];
    
    inch11[0].is_int = 1;
    inch11[1].is_int = 0;
    strcpy(inch11[1].varchar , "first record");
    inch11[0].i = 1;
    
    inch12[0].is_int = 1;
    inch12[1].is_int = 0;
    strcpy(inch12[1].varchar , "second record");
    inch12[0].i = 2;
    
    inch13[0].is_int = 1;
    inch13[1].is_int = 0;
    strcpy(inch13[1].varchar , "third record");
    inch13[0].i = 3;
    
    inch14[0].is_int = 1;
    inch14[1].is_int = 0;
    strcpy(inch14[1].varchar , "first record");
    inch14[0].i = 1;
    
    int_or_char inch21[3];
    int_or_char inch22[3];
    int_or_char inch23[3];
    
    inch21[0].is_int = 1;
    inch21[1].is_int = 1;
    inch21[2].is_int = 0;
    strcpy(inch21[2].varchar , "tab 2 data");
    inch21[0].i = 1;
    inch21[1].i = 10;
    
    inch22[0].is_int = 1;
    inch22[1].is_int = 1;
    inch22[2].is_int = 0;
    strcpy(inch22[2].varchar , "tab 2 data");
    inch22[0].i = 3;
    inch22[1].i = 20;
    
    inch23[0].is_int = 1;
    inch23[1].is_int = 1;
    inch23[2].is_int = 0;
    strcpy(inch23[2].varchar , "tab 2 data");
    inch23[0].i = 5;
    inch23[1].i = 30;
    
    
    create("tab1",2, s1,t1);
    create("tab2",3, s2,t2);
    
    
    //drop("mytable");
    //insert("tab1",2,inch11);
    //insert("tab1",2,inch12);
    //insert("tab1",2,inch13);
    //insert("tab1",2,inch14);
    
    //insert("tab2",3,inch21);
    //insert("tab2",3,inch22);
    //insert("tab2",3,inch23);
    
    char cols1[2][128] = {"","username"};
    char cols2[2][128] = {"","number"};
    char cols3[2][128] = {"","number"};
    int_or_char constant;
    constant.is_int = 1;
    constant.i = 30;
    //select_simple("",0,"tab1","",0,constant);
    //select_join(cols1,0,"tab1",cols2,0,"tab2",cols3,0,"",0,0,constant,"count",0,1,constant,"id",0,5,"id",0);
    aggregation agg[3];
    agg[1].op = 5;
    strcpy(agg[1].col_name ,"count");
    //group_simple("tab1", "id", &agg, 1, "id", 3, constant);
  
    group_join("tab1", "id",&agg, 0, "tab2","",&agg, 1, &agg,0, "count",0, 0,  constant, "count",0, 1, constant,  "id",0,  5, "id",0);
    return 0;
}
