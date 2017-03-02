
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
#include <functional>

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

//int 0
void div0 (InterruptParams par){
    bsod("1/0 is not infinity at %p", par.eip);
}

//int 6
void invalidOpcode(InterruptParams par) {
    bsod("Invalid opcode at %p", par.eip);
}

//int 8
void doublefault(InterruptParamsErr par){
    bsod("Double fault at %p\nIt may be an uncaught interruption.", par.eip);
    //should not return eip is UB.
}

//int 13
void gpfault(InterruptParamsErr par){
    bsod("General Protection fault at %p with code %x", par.eip, par.errorCode);
}

//int 14
void pagefault(InterruptParamsErr par){
    printf("Page fault at %p with code %x accessing %p\n", par.eip, par.errorCode, getCR2());
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

//int 0x21
void keyboard(InterruptParams) {
    kbd.handleScanCode(inb(0x60));
    pic.endOfInterrupt(1);
}


extern "C" void kmain(multibootInfo* multibootinfo) {
    multiboot = *multibootinfo;
    PDE[0].present = false;
    cli;
    init(); //C++ global contructors should not change machine state.
    initkmalloc();
    gdt.init();
    idt.init();
    idt.addInt(0, div0);
    idt.addInt(6, invalidOpcode);
    idt.addInt(8, doublefault);
    idt.addInt(13, gpfault);
    idt.addInt(14, pagefault);
    sti;

#define BLA 3
#if BLA == 1
    HDD first(1,true);
    first.init();

    //first.writeaddr(0xf0095,"random data !",13);
    /*char text [15] = {};
    first.readaddr (0xf0095,text,13);

    printf("Read text %s \n",text);*/

    PartitionTableEntry part1 = first[1];

    fb.printf ("Partition 1 from %8x to %8x \n",part1.begLBA,part1.endLBA);

    Partition pa1 (&first,part1);

    /*uchar buffer[512];
    pa1.readlba(0,buffer,1);

    for(int i = 0 ; i < 512 ; ++ i){
        printf("%x ",buffer[i]);
    }
    printf("\n");*/

    fat::FS fs (&pa1);

    fat::Directory* root = fs.getRootFat();
    root->load();
    Directory * boot = (*root)["boot"]->dir();
    assert(boot);
    //printf("%p\n",boot);
    Directory* grub = (*boot)["grub"]->dir();
    assert(grub);
    auto v  = grub->getFilesName();
    printf("Filenames : \n");
    for(auto s : v){
        printf("-%s;\n",s.c_str());
    }
    fb.printf("\n");
    while(true){
        for(int i = 0 ; i < 1000000 ; ++i);
        first.getStatus().printStatus();
        fb.printf("\r");
    }

#elif BLA == 3
    idt.addInt(0x21,keyboard);
    pic.activate(Pic::KEYBOARD);

    kbd.setKeymap(&azertyKeymap);

    HDD first(1,true);
    first.init();

    PartitionTableEntry part1 = first[1];

    Partition pa1 (&first,part1);

    fat::FS fs (&pa1);
    CommandLine cl;

    cl.pwd = fs.getRootFat();


    // proof of concept for command
    cl.add("test",[](CommandLine*,const vector<string>&){
                fb.printf("test Command in kmain\n");
            });

    //running command line
    cl.run();
#else
    //[insert here useful kernel code]
#endif
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
