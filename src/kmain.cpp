#include "FrameBuffer.h"
#include "utility.h"
#include "globals.h"
#include "Interrupt.h"

typedef void(*funcp)();

extern "C" {
    extern funcp __init_array_start;
    extern funcp __init_array_end;
}

void init(){
    funcp *beg = & __init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}

extern "C" void prout(){
    bsod("prout !!");
}

extern "C" void inter153();

extern "C" void kmain() {

    init();
    IDT[153].setAddr(reinterpret_cast<void*>(prout));
    IDT[153].present =true;
    lidt();
    breakpoint;
    interrupt<153>();
    bsod("You fail !!");

}
extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
