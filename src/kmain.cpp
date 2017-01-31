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

extern "C" void bigfail(){
    bsod("Double fault :-(");
}

extern "C" void inter153();

extern "C" void keyboard() {
    bsod("Key pressed!");
    breakpoint;
    pic.endOfInterrupt(0);
}

extern "C" void kmain() {

    init();
    IDT[8].setAddr(reinterpret_cast<void*>(bigfail));
    IDT[8].present =true;
    IDT[0x21].setAddr(reinterpret_cast<void*>(keyboard));
    IDT[0x21].present =true;
    //pic.clearMask(1);
    asm("sti");
    lidt();
    fb.puts("Salut l'ami !!!\n");
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
