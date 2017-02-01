#include "FrameBuffer.h"
#include "utility.h"
#include "globals.h"
#include "Interrupt.h"
#include "Segmentation.h"
#include "Keyboard.h"

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
    Keyboard kb;
    kb.setKeymap(&dvorakKeymap);
    kb.handleScanCode(inb(0x60));
    Keycode kc = kb.poll();
    bsod("Key pressed! %x %c %x %d", kc.scanCode, kc.symbol, kc.flags, kc.isRelease);
    pic.endOfInterrupt(1);
}

extern "C" void kmain() {
    init();
    cli;
    stdGDT();
    lgdt();
    switchSegReg();
    IDT[8].setAddr(reinterpret_cast<void*>(bigfail));
    IDT[8].present =true;
    IDT[0x21].setAddr(reinterpret_cast<void*>(keyboard));
    IDT[0x21].present =true;
    lidt();
    pic.activate(Pic::KEYBOARD);
    asm("sti");
    fb.puts("Salut l'ami !!!\n");
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
