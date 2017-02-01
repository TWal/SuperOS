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

extern "C" void keyboard() {
    Keyboard kb;
    kb.setKeymap(&dvorakKeymap);
    kb.handleScanCode(inb(0x60));
    Keycode kc = kb.poll();
    bsod("Key pressed! %x %c %x %d", kc.scanCode, kc.symbol, kc.flags, kc.isRelease);
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

    idt.addInt(153,printer);
    idt.addInt(227,printer);
    idt.addInt(0,div0);
    idt.addInt(200,sum);
    interrupt<153>(12,45);
    interrupt<153>(13,46);
    interrupt<153>(14,47);
    interrupt<227>(-98,456);
    interrupt<227>(0,interruptr<200>(40,2));
    // int i = 1000000000;
    /*while (true){
        volatile int j = 1/i;
        (void)j;
        }*/


    // asm("sti");
    //lidt();
    fb.puts("Salut l'ami !!!\n");
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
