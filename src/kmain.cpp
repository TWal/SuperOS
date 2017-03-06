
#include "IO/FrameBuffer.h"
#include "utility.h"
#include "Interrupts/Interrupt.h"
#include "Memory/Segmentation.h"
#include "IO/Keyboard.h"
#include "Memory/Paging.h"
#include "HDD/HardDrive.h"
#include "Memory/PhysicalMemoryAllocator.h"
#include "HDD/FAT.h"
#include "Memory/dirtyMalloc.h"
#include <stdarg.h>
#include <memory>
#include "IO/CommandLine.h"
#include <functional>
#include "multiboot.h"
#include "Interrupts/Pic.h"
#include "HDD/Ext2.h"

using namespace std;

multibootInfo multiboot;

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
void div0 (const InterruptParams& par){
    bsod("1/0 is not infinity at %p", par.eip);
}

//int 6
void invalidOpcode(const InterruptParams& par) {
    bsod("Invalid opcode at %p", par.eip);
}

//int 8
void doublefault(const InterruptParamsErr& par){
    bsod("Double fault at %p\nIt may be an uncaught interruption.", par.eip);
    //should not return eip is UB.
}

//int 13
void gpfault(const InterruptParamsErr& par){
    bsod("General Protection fault at %p with code %x", par.eip, par.errorCode);
}

//int 14
void pagefault(const InterruptParamsErr& par){
    printf("Page fault at %p with code %x accessing %p\n", par.eip, par.errorCode, getCR2());
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

//int 0x21
void keyboard(const InterruptParams&){
    kbd.handleScanCode(inb(0x60));
    pic.endOfInterrupt(1);
}


extern "C" void kmain(multibootInfo* multibootinfo) {
    multiboot = *multibootinfo;
    PDE[0].present = false; // desactivate identity mapping;
    cli; // clear interruption
    init(); //C++ global contructors should not change machine state.
    initkmalloc(); // malloc initialisation
    gdt.init(); // segmentation initialisation
    idt.init(); // interruption initialisation
    idt.addInt(0, div0); // adding various interruption handlers
    idt.addInt(6, invalidOpcode);
    idt.addInt(8, doublefault);
    idt.addInt(13, gpfault);
    idt.addInt(14, pagefault);
    sti; // enable interruption

#define BLA 0
#define EMUL // comment for LORDI version
#if BLA == 0
    HDD first(1,true);
    first.init();
    PartitionTableEntry part1 = first[1];
    fb.printf ("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    Partition pa1 (&first,part1);
    Ext2::FS fs (&pa1);

    idt.addInt(0x21,keyboard); // register keyborad interrrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interrruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.

    CommandLine cl;
    cl.pwd = fs.getRoot();
    cl.run();

#elif BLA == 1
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



#else // ---------------NON-TEST CODE----------------------

    idt.addInt(0x21,keyboard); // register keyborad interrrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interrruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.

    HDD first(1,true); // load HHD manlipulation structure
    first.init(); // load MBR and partitions

#ifdef EMUL

    PartitionTableEntry part1 = first[1];
    Partition pa1 (&first,part1); // load first partition

#else

    const PartitionTableEntry *part1 = first.partWithPred([](const Partition&part){
            return fat::CheckUUID(0x04728457,part);
            });
    if(part1 == nullptr)bsod("Main partition not found");
    Partition pa1 (&first,*part1); // load partition with FAT UUID 04728457

#endif

    fat::FS fs (&pa1); // load file system FAT on the opened partition
    CommandLine cl; // load Command line

    cl.pwd = fs.getRootFat(); // set root directory as pwd on boot.

    cl.run(); //run command line
#endif
    while(true) asm volatile("cli;hlt");
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
