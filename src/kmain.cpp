#include "FrameBuffer.h"
#include "utility.h"
#include "globals.h"
#include "Interrupt.h"
#include "Segmentation.h"
#include "Keyboard.h"
#include "Paging.h"
#include "HardDrive.h"
#include "PhysicalMemoryAllocator.h"
#include "FAT.h"

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

extern "C" void kmain(multibootInfo* multibootinfo) {
    multiboot = *multibootinfo;
    PDE[0].present = false;
    cli;
    init();
    gdt.init();
    idt.init();
    idt.addInt(8,doublefault);
    sti;
    idt.addInt(0,div0);


#define BLA 1
#if BLA == 0
    fb.printf("Memory available: %dkb\n", multiboot.mem_upper);
    void* p1 = physmemalloc.alloc();
    void* p2 = physmemalloc.alloc();
    void* p3 = physmemalloc.alloc();
    physmemalloc.free(p1);
    void* p4 = physmemalloc.alloc();
    physmemalloc.free(p2);
    void* p5 = physmemalloc.alloc();
    fb.printf("%x %x %x %x %x\n", p1, p2, p3, p4, p5);

#elif BLA == 1
    HDD first(1,true);
    first.init();

    first.writeaddr(0xf0095,"random data !",13);

    PartitionTableEntry part1 = first[1];

    fb.printf ("Partition 1 from %8x to %8x \n",part1.begLBA,part1.endLBA);

    Partition pa1 (&first,part1);
    FATFS fs (&pa1);

    fb.printf("\n");
    while(true){
        for(int i = 0 ; i < 1000000 ; ++i);
        first.getStatus().printStatus();
        fb.printf("\r");
    }

#elif BLA == 2
    volatile lint i = 42000000000000;
    volatile lint j = 10000000000;
    int res = i/j; // testing libgcc
    fb.printf ("val :%d ",res);
#else
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
#endif
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
