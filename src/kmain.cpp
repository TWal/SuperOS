
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
#include "HDD/VFS.h"
#include "IO/OSFileDesc.h"
#include "../src32/Graphics.h"
#include "Graphics/Screen.h"

#include<vector>
#include<string>
#include<set>
#include<deque>

using namespace std;
using namespace video;

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
    printf("1/0 is not infinity at %p", par.rip);
    stop;
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
#define GRAPH_TEST 6


#define NO_TEST 42

//-----------------------------------kinit---------------------------

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
#ifdef UNITTEST
    unitTest = true;
#endif
    fprintf(stderr,"Kernel Starting\n"); // TODO lots of logging.
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
    paging.allocStack(kargs->stackAddr,10); // allocation of kernel stack (fixed size )
    u64 fontPtr = kargs->font;
    asm volatile(
        "and $0xFFF,%rsp; sub $0x1000,%rsp"
        ); // rsp switch : all stack pointer are invalidated (kargs for example);
    /*asm volatile(
        "and $0xFFF,%rbp; sub $0x1000,%rbp"
        );*/ // rbp switch : use this code only in O0, gcc can use rbp for other thing in O123.

    //Graphical Initializing
    GraphicalParam* gp = (GraphicalParam*)kargs->GraphicalParam;
    fprintf(stderr,"Kernel Graphics : %d * %d\n",gp->Xsize,gp->Ysize);
    video::screen.init(gp);


    paging.removeIdent(); // remove the identity paging necessary for boot process
    gdt.initkernelTLS(1); // Initialize kernel TLS with 1 page.
    kheap.init(&kernel_code_end);// creating kernel heap
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

    printf("\n64 bits kernel booted!! built on %s at %s \n",__DATE__,__TIME__);

#define BLA GRAPH_TEST
#define EMUL // comment for LORDI version
#if BLA == TMP_TEST

#elif BLA == GRAPH_TEST
    printf("fontPtr %p\n",fontPtr);
    Font* font = pageHeap.alloc<Font>(fontPtr,2);
    font->init();
    screen.putChar('H',0,0,*font,Color::white,Color::black);
    screen.putChar('e',8,0,*font,Color::white,Color::black);
    screen.putChar('l',16,0,*font,Color::white,Color::black);
    screen.putChar('l',24,0,*font,Color::white,Color::black);
    screen.putChar('o',32,0,*font,Color::white,Color::black);
    screen.putChar('W',48,0,*font,Color::white,Color::black);
    screen.putChar('o',56,0,*font,Color::white,Color::black);
    screen.putChar('r',64,0,*font,Color::white,Color::black);
    screen.putChar('l',72,0,*font,Color::white,Color::black);
    screen.putChar('d',80,0,*font,Color::white,Color::black);
    screen.putChar('!',88,0,*font,Color::white,Color::black);
    for(int i = 0 ; i < 1024 ; ++i){
        //video::screen.clear();
        screen.set(i,18,Color::white);
        screen.send();
    }

#elif BLA == USER_TEST
    breakpoint;
    HDD::HDD first(1,true);
    first.init();
    //PartitionTableEntry part1 = first[1];
    //fb.printf ("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    //Partition pa1 (&first,part1);
#ifdef EMUL

    HDD::PartitionTableEntry part1 = first[1];
    HDD::Partition pa1 (&first,part1); // load first partition

#else

    const HDD::PartitionTableEntry *part1 = first.partWithPred([](const HDD::Partition&part){
            //u32 UUID[4] = {0xe9be36b1,0x894793a2,0x32db3294,0xa907a74a};
            u32 UUID[4]= {0x1e0a2799,0x9e4e1c52,0x1868dc89,0x0397fcb7};
            return HDD::Ext2::CheckUUID(UUID,part);
        });
    if(part1 == nullptr)bsod("Main partition not found");
    HDD::Partition pa1 (&first,*part1); // load partition with FAT UUID 04728457
#endif

    HDD::Ext2::FS fs (&pa1);

    ProcessInit();

    HDD::File* initf = (*(fs.getRoot()))["init"];
    assert(initf);
    assert(initf->getType() == HDD::FileType::RegularFile);
    HDD::RegularFile* init = dynamic_cast<HDD::RegularFile*>(initf);
    ProcessGroup pg(1);
    Process initp(1,&pg);
    Thread* initt = initp.loadFromBytes(init);
    FBStream* s= new FBStream();
    FileDescriptor fd(s);
    printf("Init process %p\n",&initp);
    initp._fds.push_back(FileDescriptor());
    initp._fds.push_back(fd);
    schedul.init(initt);
    schedul.run();



#elif BLA == EXT2_TEST

    HDD::HDD first(1,true);
    first.init();
    HDD::PartitionTableEntry part1 = first[1];
    HDD::PartitionTableEntry part2 = first[2];
    printf("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    printf("Partition 2 beginning at %8x of size %8x \n",part2.begLBA,part2.size);
    HDD::Partition pa1(&first,part1);
    HDD::Partition pa2(&first,part2);
    HDD::Ext2::FS fs1(&pa1);
    HDD::Ext2::FS fs2(&pa2);
    HDD::VFS::FS fs(&fs1);

    HDD::VFS::File* f = fs.vgetRoot()->get("mnt");
    assert(f != nullptr);
    assert(f->getType() == HDD::FileType::Directory);
    HDD::VFS::Directory* d = dynamic_cast<HDD::VFS::Directory*>(f);
    fs.mount(d, &fs2);

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


/**
   @mainpage Super OS Documentation

   Welcome to the Super OS documentation, WIP, good luck !
 */



extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
extern "C" void  __cxa_atexit(){}
