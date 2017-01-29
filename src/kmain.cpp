#include "FrameBuffer.h"
#include "utility.h"
#include "globals.h"

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

extern "C" void kmain() {
    init();
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < i; ++j) {
            fb.putc(' ');
        }
        fb.printf("%s%c %d\f", "Hello World", '!', i);
    }

    while(true) {
        for(int i = 0; i < 5000000; ++i);
        fb.scroll(1);
    }
}
extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
extern "C" void  __cxa_atexit(){}
