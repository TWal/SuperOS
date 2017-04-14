
#include "IO/FrameBuffer.h"
#include "utility.h"
#include "../src32/KArgs.h"
#include "Interrupts/Interrupt.h"
#include "IO/Keyboard.h"
#include "Memory/Paging.h"
#include "Memory/Segmentation.h"
#include "HDD/HardDrive.h"
#include "Memory/PhysicalMemoryAllocator.h"
//#include "HDD/FAT.h"
#include <stdarg.h>
#include "IO/CommandLine.h"
//#include <functional>
#include "Interrupts/Pic.h"
#include "HDD/Ext2.h"
#include "Memory/Heap.h"
#include <stdlib.h>
#include "IO/Serial.h"
#include "Interrupts/TaskSegment.h"
#include "User/Syscall.h"
#include "Processes/Scheduler.h"
#include "Bitset.h"
#include "User/Context.h"
#include "Memory/PageHeap.h"

#include<vector>
#include<string>
#include<set>
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


/// C++ global constructors initialization.
void init(){
    funcp *beg = &__init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}

/// Division by 0 exception handler
void div0 (const InterruptParams& par){
    bsod("1/0 is not infinity at %p", par.rip);
}

/// Invalid opcode exception handler
void invalidOpcode(const InterruptParams& par) {
    bsod("Invalid opcode at %p", par.rip);
}

/// Double fault exception handler
void doublefault(const InterruptParamsErr& par){
    bsod("Double fault at %p\nIt may be an uncaught interruption.", par.rip);
    //should not return, rip is UB.
}

/// General protection fault exception handler
void gpfault(const InterruptParamsErr& par){
    printf("General Protection fault at %p with code %x", par.rip, par.errorCode);
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

/**
   @brief Page fault exception handler
   @todo Not crash on user page fault
*/

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
#define USER_TEST 5


#define NO_TEST 42

//-----------------------------------kmain---------------------------

#ifdef UNITTEST
void unittest();
#endif

/**
    @brief This is the entry point of the kernel

    @param kargs : The description of memory state after loading.

    It calls initialization routines, do startup configuration (like setting exception handlers) and
    then, when the kernel is operational, it calls the Scheduler method run.

*/

// WARNING : kinit local variables should not exceed 2K for stack switching
extern "C" [[noreturn]] void kinit(KArgs* kargs) {
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
    paging.allocStack((void*)kargs->stackAddr,10); // allocation of kernel stack (fixed size )
    asm volatile(
        "and $0xFFF,%rsp; sub $0x1000,%rsp"
        ); // rsp switch : all stack pointer are invalidated (kargs for example);
    /*asm volatile(
        "and $0xFFF,%rbp; sub $0x1000,%rbp"
        );*/ // rbp switch : use this code only in O0, gcc can use rbp for other thing in O123.
    paging.removeIdent(); // remove the identity paging necessary for boot process
    __setbrk(kheap.init(&kernel_code_end));// creating kernel heap
    __initmalloc(); // initializing kernel malloc : no heap access before this point.
    pageHeap.init(); // initializing page Heap to map physical pages on will.
    // (i.e a heap with 4K aligned malloc).
    syscallInit(); // intialize syscall API
    tss.load(); // load TSS for enabling interrupts from usermode
    tss.RSP[0] = nullptr; // the kernel stack really start from 0.

#ifdef UNITTEST
    unittest();
    stop;
#endif

#define BLA USER_TEST
#define EMUL // comment for LORDI version
#if BLA == TMP_TEST

#elif BLA == USER_TEST
    fb.printf("64 bits kernel booted, paging, stack and heap initialized!!\n");
    breakpoint;
    HDD first(1,true);
    first.init();
    PartitionTableEntry part1 = first[1];
    //fb.printf ("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    Partition pa1 (&first,part1);
    Ext2::FS fs (&pa1);

    File* init = (*(fs.getRoot()))["init"];
    assert(init);
    ProcessGroup pg(1);
    Process initp(1,&pg);
    Thread* initt = initp.loadFromBytes(init);
    schedul.init(initt);
    schedul.run();



#elif BLA == EXT2_TEST

    HDD first(1,true);
    first.init();
    PartitionTableEntry part1 = first[1];
    fb.printf ("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    Partition pa1 (&first,part1);
    Ext2::FS fs (&pa1);

    idt.addInt(0x21,keyboard); // register keyborad interrrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interrruption
    //kbd.setKeymap(&azertyKeymap); // activate azerty map.
    kbd.setKeymap(&dvorakKeymap); // activate dvorak map.

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

    kend();
}




/**
   @brief This function is called as often as possible by scheduler to update the kernel

   No large code in this function should run at each call. it is better to just make a
   condition check and maybe do something if the condition is met

   @todo Do something in kloop like event handling

 */

void kloop(){
}




/**
   @brief This function is called to end the kernel, and may, one day, shutdown the computer
*/
[[noreturn]] void kend(){
    cli;
    //fb.clear();
    printf("\n\nKernel is ready to Shutdown, please press power button for 5 sec");
    while(true) stop;
}




extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
extern "C" void  __cxa_atexit(){}
