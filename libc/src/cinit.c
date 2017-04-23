#include "../string.h"
void __initmalloc();

extern char __bss_start;
extern char _end;
void __cinit(){ // all c runtime initialization
    char *beg = &__bss_start, *end = &_end;
    memset(beg,0,end-beg);
    __initmalloc();
}

//#ifdef SUP_OS_KERNEL

//#include "assert.h"

void __stack_chk_fail(){
    //bsod("Stack Overflow on libsupc++");
}

//#endif
