void __initmalloc();

void __cinit(){ // all c runtime initialization
    __initmalloc();
}

//#ifdef SUP_OS_KERNEL

//#include "assert.h"

void __stack_chk_fail(){
    //bsod("Stack Overflow on libsupc++");
}

//#endif
