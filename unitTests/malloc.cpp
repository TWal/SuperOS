#include"../src/utility.h"
#include <stdlib.h>

void unittest(){
    char* p0 = (char*)sbrk(0);
    char* p1 = (char*)malloc(42); // alloc 8 aligned : 48
    char* p2 = (char*)malloc(500); // align 8 : 504
    char* p3 = (char*)malloc(0x4000);
    pbool(p1 == p0, "Malloc start at first place");
    pbool(p1 + 48 +8 == p2);
    pbool(p2 + 504 + 8 == p3);
    free(p2);
    char* p4 = (char*) malloc(0x3000);
    pbool(p3 + 0x4000 +8 == p4, "Malloc do not use a block again if it is too big");
    char* p5 = (char*)malloc(12); // 8 aligned : 16
    pbool(p5 == p2,"But it can use it again if small enough");
    char* p6 = (char*)malloc(400);
    pbool(p5 + 16 +8 == p6,"Even twice if needed");
    char* end = (char*)sbrk(0);
    pbool(end == p0 +4 * 8 + 48 +504 + 0X4000 + 0x3000, "Malloc allocate the right amount on Heap");
    free(p3);
    pbool(end == sbrk(0),"Do not free Heap if impossible");
    free(p4);
    pbool(sbrk(0) == 3*8 +48 + 16 +400 + p0,"But free it if needed");
}

