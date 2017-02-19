
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
#include "dirtyMalloc.h"
#include <stdarg.h>
#include <memory>
#include "CommandLine.h"

using namespace std;

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
    bsod("Double fault !! It may be an uncaught interruption.");
}
void pagefault(int a, int b){
    (void)a;
    (void)b;
    bsod("Page fault !! Access on invalid address");
// TODO get address from CR2 + read error code
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
    bsod(" 1/0 is not infinity !");
}

int sum (int a, int b){
    return a+b;
}

extern "C" void kmain(multibootInfo* multibootinfo) {
    multiboot = *multibootinfo;
    PDE[0].present = false;
    cli;
    init(); //C++ global contructors should not change machine state.
    gdt.init();
    idt.init();
    idt.addInt(8,doublefault);
    idt.addInt(0xE,pagefault);
    sti;
    idt.addInt(0,div0);

#define BLA 4
#if BLA == 0
    fb.printf("Memory available: %dkb\n", multiboot.mem_upper);
    uint size = 1024*1024*8;
    char* p1 = (char*)kmalloc(size);
    // the size where the bsod bug

    for(uint i = 0; i < size; ++i) {
        p1[i] = 0;
    }
    bsod("Coucou");

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
#elif BLA == 3
    std::string s = "/prout/caca/hey";
    std::string s2 = s;
    fb.printf("%s\n",s2.c_str());
    auto v = split(s,'/');
    for(auto s : v){
        fb.printf("%s\n",s.c_str());
    }
    fb.printf("%u\n",v.size());

    fb.printf("%s\n",concat(v,'/').c_str());

    //v.at(45);

#elif BLA == 4
    idt.addInt(0x21,keyboard);
    pic.activate(Pic::KEYBOARD);

    kbd.setKeymap(&azertyKeymap);

    CommandLine cl;

    // proof of concept for command
    cl.add("test",[](const vector<string>& args){
                fb.printf("test Command in kmain\n");
            });

    //running command line
    cl.run();


#else
    volatile int  j =42;
    breakpoint;
    j-=42;
    //volatile int i = 1/j;
    idt.addInt(0x21,keyboard);
    pic.activate(Pic::KEYBOARD);

    kbd.setKeymap(&azertyKeymap);

    while(true) {
        breakpoint;
        breakpoint;
        Keycode kc = kbd.poll();
        breakpoint;
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
