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


void doublefault(int a, int b){
    (void)a;
    (void)b;
    bsod("Double fault !!\n It may be an uncaught interruption");
}

void printer (int a,int b){
    fb.printf ("a is : %d, b is %d\n",a,b);
}

void keyboard(int a, int b) {
    (void)a;
    (void)b;
    kbd.handleScanCode(inb(0x60));
    pic.endOfInterrupt(1);
}

void div0 (int a, int b){
    (void)a;
    (void)b;
    bsod(" 1/0 in not infinity !");
}

int sum (int a, int b){
    return a+b;
}

extern "C" void kmain() {
    cli;
    init();
    gdt.init();
    idt.init();
    idt.addInt(8,doublefault);
    sti;
    idt.addInt(0,div0);
    idt.addInt(0x21,keyboard);
    pic.activate(Pic::KEYBOARD);

    kbd.setKeymap(&azertyKeymap);

    while(true) {
        Keycode kc = kbd.poll();
        if(!kc.isRelease && kc.symbol > 0) {
            if(kc.symbol == '\b') {
                fb.puts("\b \b");
            } else {
                fb.putc(kc.symbol);
            }
        }
        //fb.printf("Key pressed! %x %x %c %x %d\n", kc.scanCode, kc.symbol, kc.symbol, kc.flags, kc.isRelease);
    }
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
