
#include "IO/FrameBuffer.h"
#include "utility.h"
#include "../src32/KArgs.h"
#include "Interrupts/Interrupt.h"
#include "IO/Keyboard.h"
#include "Memory/Paging.h"
#include "Memory/Segmentation.h"
#include "HDD/HardDrive.h"
#include "Memory/PhysicalMemoryAllocator.h"
#include "HDD/FAT.h"
#include <stdarg.h>
#include "IO/CommandLine.h"
//#include <functional>
#include "Interrupts/Pic.h"
#include "HDD/Ext2.h"
#include "Memory/Heap.h"
#include <stdlib.h>
#include "IO/Serial.h"
#include "User/Context.h"
#include "Interrupts/TaskSegment.h"
#include "User/Syscall.h"

#include<vector>
#include<string>
#include<deque>

using namespace std;

//multibootInfo multiboot;

typedef void(*funcp)();

static_assert(sizeof(funcp) == 8);

extern "C" {
    extern funcp __init_array_start;
    extern funcp __init_array_end;
}

extern "C" void* kernel_code_end;

void init(){
    funcp *beg = &__init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}

//int 0
void div0 (const InterruptParams& par){
    bsod("1/0 is not infinity at %p", par.rip);
}

//int 6
void invalidOpcode(const InterruptParams& par) {
    bsod("Invalid opcode at %p", par.rip);
}

//int 8
void doublefault(const InterruptParamsErr& par){
    bsod("Double fault at %p\nIt may be an uncaught interruption.", par.rip);
    //should not return, rip is UB.
}

//int 13
void gpfault(const InterruptParamsErr& par){
    printf("General Protection fault at %p with code %x", par.rip, par.errorCode);
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

//int 14
void pagefault(const InterruptParamsErr& par){
    printf("Page fault at %p with code %x accessing %p\n", par.rip, par.errorCode, getCR2());
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

//int 0x21
void keyboard(const InterruptParams&){
    kbd.handleScanCode(inb(0x60));
    pic.endOfInterrupt(1);
}

void dummy(const InterruptParams&){

}

void hello(const InterruptParams&){
    printf("hello system call");
}


//--------------------TESTING MACRO
#define TMP_TEST (-1)
#define FAT_TEST 1
#define KBD_TEST 2
#define CL_TEST 3
#define EXT2_TEST 4


#define NO_TEST 42

//-----------------------------------kmain---------------------------

#ifdef UNITTEST
void unittest();
#endif

extern "C" void kmain(KArgs* kargs) {
    cli; // clear interruption
    init(); //C++ global constructors should not change machine state.
    gdt.init();
    idt.init(); // interruption initialization
    idt.addInt(0, div0); // adding various interruption handlers
    idt.addInt(6, invalidOpcode);
    idt.addInt(8, doublefault);
    idt.addInt(13, gpfault);
    idt.addInt(14, pagefault);
    sti; // enable interruption
    // initializing memory allocation by marking occupied Areas (code + stack + paging).
    physmemalloc.init((void*)kargs->freeAddr,kargs->RAMSize,
                      (OccupArea*)kargs->occupArea,kargs->occupAreaSize);
    paging.init((PageEntry*)kargs->PML4); // initializing paging
    paging.allocStack((void*)kargs->stackAddr,3); // allocation kernel stack (fixed size for now)
    asm volatile(
        "and $0xFFF,%rsp; sub $0x1000,%rsp"
        ); // rsp switch : all stack pointer are invalidated (kargs for example);
    /*asm volatile(
        "and $0xFFF,%rbp; sub $0x1000,%rbp"
        );*/ // rbp switch : use this code only in O0, gcc can use rbp for other thing in O123.
    paging.removeIdent();
    kheap.init(&kernel_code_end);
    initmalloc();

#ifdef UNITTEST
    unittest();
    stop;
#endif

#define BLA TMP_TEST
#define EMUL // comment for LORDI version
#if BLA == TMP_TEST
    fb.printf("64 bits kernel booted, paging, stack and heap initialized!!\n");
    breakpoint;

    syscallInit();

    tss.load();
    tss.RSP[0] = (void*)0xFFFFFFFFFFFFF800ull;

    testLaunchCPL3();


#elif BLA == EXT2_TEST

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

#elif BLA == FAT_TEST
    HDD first(1,true);
    first.init();

    //first.writeaddr(0xf0095,"random data !",13);
    /*char text [15] = {};
    first.readaddr (0xf0095,text,13);

    printf("Read text %s \n",text);*/

    PartitionTableEntry part1 = first[1];

    fb.printf ("Partition 1 from %8x to %8x \n",part1.begLBA,part1.size);

    Partition pa1 (&first,part1);

    uchar buffer[512];
    pa1.readlba(0,buffer,1);

    /*for(int i = 0 ; i < 512 ; ++ i){
        printf("%x ",buffer[i]);
    }
    printf("\n");*/

    fat::FS fs (&pa1);

    fat::Directory* root = fs.getRootFat();
    root->load();
    Directory * boot = (*root)["boot"]->dir();
    assert(boot);
    printf("%p\n",boot);
    Directory* grub = (*boot)["grub"]->dir();
    assert(grub);
    /*auto v  = grub->getFilesName();
    printf("Filenames : \n");
    for(auto s : v){
        printf("-%s;\n",s.c_str());
        }*/
    fb.printf("\n");
    while(true){
        for(int i = 0 ; i < 1000000 ; ++i);
        first.getStatus().printStatus();
        fb.printf("\r");
    }

#elif BLA == KBD_TEST
    idt.addInt(0x21,keyboard); // register keyborad interrrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interrruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.
    breakpoint;

    while(true) {
        auto k = kbd.poll();
        if(!k.isRelease)printf("%c",k.symbol);}


#elif BLA == CL_TEST
    idt.addInt(0x21,keyboard); // register keyboard interrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.
    CommandLine cl; // load Command line
    cl.run(); //run command line
    stop;




#else // ---------------NON-TEST CODE----------------------

    idt.addInt(0x21,keyboard); // register keyboard interrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.

    HDD first(1,true); // load HHD manipulation structure
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

    printf("kmain end : this is not normal for an operating system !");
    stop;
}

extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
//extern "C" void  __cxa_atexit(){}
