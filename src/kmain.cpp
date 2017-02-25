
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

extern "C" void* kernel_code_end;

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

void gpfault(int a, int b){
    (void)a;
    (void)b;
    bsod("General Protection fault !!");
}

void pagefault(int a, int b){
    (void)a;
    (void)b;
    printf("Page fault\n");
    while(true) asm volatile("cli;hlt");
    //bsod("Page fault! (aka. segmentation fault)");
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
    initkmalloc();
    gdt.init();
    idt.init();
    idt.addInt(0,div0);
    idt.addInt(8,doublefault);
    idt.addInt(14,pagefault);
    idt.addInt(13,gpfault);
    sti;

#define BLA 5
#if BLA == 0

    char* p1 = (char*)malloc(13);
    char* p2 = (char*)malloc(19);
    char* p3 = (char*)malloc(23);
    char* p4 = (char*)malloc(27);
    fb.printf("%x %x %x %x\n", p1, p2, p3, p4);
    free(p2);
    char* p5 = (char*)malloc(21);
    fb.printf("%x\n", p5);
    while(1) asm volatile ("cli;hlt");

#elif BLA == 1
    HDD first(1,true);
    first.init();

    first.writeaddr(0xf0095,"random data !",13);

    PartitionTableEntry part1 = first[1];

    fb.printf ("Partition 1 from %8x to %8x \n",part1.begLBA,part1.endLBA);

    Partition pa1 (&first,part1);
    //fat::FS fs (&pa1);

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
    idt.addInt(0x21,keyboard);
    pic.activate(Pic::KEYBOARD);

    kbd.setKeymap(&azertyKeymap);

    printf("%p",&kernel_code_end);
    map<std::string,int> m;
    m["un"] = 1;
    char s[] = "hey !fsqfsdsf";

    fb.puts(s);
    s[6] = 0;
    fb.puts(s);
    breakpoint;

    CommandLine cl;

    // proof of concept for command
    cl.add("test",[](const vector<string>& args){
                fb.printf("test Command in kmain\n");
            });

    //running command line
    cl.run();
#elif BLA == 5
    map<std::string,int> m;
    m["z"] = 1;
    //char s[] = "hey !fsqfsdsf";

    //fb.puts(s);
    //s[6] = 0;
    //fb.puts(s);
    //breakpoint;
    m["y"] = 0;
    m["x"] = 2;
    m["w"] = 3;
    m["v"] = 4;
    m["g"] = 4;
    m["d"] = 4;
    m["b"] = 4;
    m["aaz"] = 42;
    //breakpoint;
    for(auto p : m){
        fb.printf("(%s,%d)",p.first.c_str(),p.second);
        }


#else
    //[insert here useful kernel code]
#endif
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
