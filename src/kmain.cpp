
#include "IO/FrameBuffer.h"
#include "utility.h"
#include "../src32/KArgs.h"
#include "Interrupts/Interrupt.h"
#include "IO/Keyboard.h"
#include "Memory/Paging.h"
#include "Memory/Segmentation.h"
#include "HDD/HardDrive.h"
#include "Memory/PhysicalMemoryAllocator.h"
#include <stdarg.h>
#include "IO/CommandLine.h"
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
#include "HDD/RamFS.h"
#include "HDD/DevFS.h"
#include "Random.h"
#include "../src32/Graphics.h"
#include "Graphics/Screen.h"
#include "Graphics/Workspace.h"
#include "Graphics/TextWindow.h"
#include "Streams/PrefixStream.h"
#include "Streams/OutMixStream.h"
#include "log.h"
#include "IO/Mouse.h"

#include<vector>
#include<string>
#include<set>
#include<deque>

using namespace std;
using namespace video;
using namespace input;

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
    bsod("General Protection fault at %p with code %x", par.rip, par.errorCode);
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

/**
   @brief Page fault exception handler
   @todo Not crash on user page fault
*/
void pagefault(const InterruptParamsErr& par){
    bsod("Page fault at %p with code %x accessing %p\n", par.rip, par.errorCode, getCR2());
    breakpoint;
    while(true) asm volatile("cli;hlt");
}

//int 0x21
void keyboard(const InterruptParams&){
    kbd.handleScanCode(inb(0x60));
    //debug(Kbd,"And second %x",inb(0x60));
    pic.endOfInterrupt(1);
}

//int 0x2c
void mouseint(const InterruptParams&){
    mouse.handleByte(inb(0x60));
    pic.endOfInterrupt(12);
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
    //fprintf(stderr,"Kernel Starting\n"); // TODO lots of logging.
    info("Kernel Starting");
    info("Kernel built on %s at %s",__DATE__,__TIME__);
    cli; // clear interruption
    init(); //C++ global constructors should not change machine state.

    info(Init,"Setup segmentation");
    gdt.init();
    info(Init,"Setup interrupts");
    idt.init();
    idt.addInt(0, div0); // adding various interruption handlers
    idt.addInt(6, invalidOpcode);
    idt.addInt(8, doublefault);
    idt.addInt(13, gpfault);
    idt.addInt(14, pagefault);
    info(Init,"Enabling interrupts");
    sti;
    // initializing memory allocation by marking occupied Areas (code + stack + paging).
    info(Init,"Initializing physical memory allocator");
    physmemalloc.init((void*)kargs->freeAddr,kargs->RAMSize,
                      (OccupArea*)kargs->occupArea,kargs->occupAreaSize);
    info(Init,"Initializing paging");
    paging.init((PageEntry*)kargs->PML4);
    debug(Init,"Allocation of kernel Stack of %d pages",10);
    paging.allocStack(kargs->stackAddr,10);
    u64 fontPtr = kargs->font;
    u64 phyLoaderLogBuffer = kargs->logBuffer;
    u64 loaderPosInBuffer = kargs->posInLogBuffer;

    debug(Init,"Switching stack %d",10);
    asm volatile(
        "and $0xFFF,%rsp; sub $0x1000,%rsp"
        ); // rsp switch : all stack pointer are invalidated (kargs for example);
    /*asm volatile(
        "and $0xFFF,%rbp; sub $0x1000,%rbp"
        );*/ // rbp switch : use this code only in O0, gcc can use rbp for other thing in O123.
    barrier;

    info(Init,"Loading Graphical system/Screen");
    GraphicalParam* gp = (GraphicalParam*)kargs->GraphicalParam;
    video::screen.init(gp);
    Workspace::init();

    info(Init,"Removing identity mapping");
    paging.removeIdent();

    info(Init,"Setup heaps");
    debug(Init,"Intializing TLS with %d pages",1);
    gdt.initkernelTLS(1);
    debug(Init,"Setup kernel heap");
    kheap.init(&kernel_code_end);
    debug(Init,"Initializing malloc");
    __initmalloc(); // initializing kernel malloc : no heap access before this point.
    debug(Init,"Setup page heap");
    pageHeap.init(); // initializing page Heap to map physical pages on will.
    // (i.e a heap with 4K aligned malloc).


    //Kernel Logging system initialization.
    info(Init,"Setup user mode");
    //User Mode initialization
    debug(Init,"Setup Syscalls");
    syscallInit(); // intialize syscall API
    debug(Init,"Setup TSS");
    tss.load(); // load TSS for enabling interrupts from usermode
    tss.RSP[0] = nullptr; // the kernel stack really start from 0.

    Font::defInit(fontPtr);

    Vec2u ssize = screen.getSize();

    TextWindow* kernelLog = new TextWindow(Vec2u{ssize.x/2,0},Vec2u{ssize.x/2,ssize.y},Font::def);
    kernelLog->allowInput = false;
    TextWindow* kernelCmd = new TextWindow(Vec2u{0,0},Vec2u{ssize.x/2,ssize.y},Font::def);
    Workspace::get(0).addWin(kernelLog);
    Workspace::get(0).addWin(kernelCmd);
    kernelLog->show();
    kernelCmd->show();

    char* loaderLogBuffer = pageHeap.alloc<char>(phyLoaderLogBuffer,LOADERBUFFER);
    debug(Init,"loader log size %lld",loaderPosInBuffer);
    kernelLog->bwrite(loaderLogBuffer,loaderPosInBuffer);

    info(Init,"Workspace 0 initialization");
    OSStreams.resize(4);
    SerialStream* sers = new SerialStream();
    OutMixStream* log = new OutMixStream({sers,kernelLog});
    PrefixStream* cmd2log = new PrefixStream(log,"[Cmd Out]  ");
    OutMixStream* cmdOut = new OutMixStream({cmd2log,kernelCmd});
    OSStreams[0] = kernelCmd;
    OSStreams[1] = cmdOut;
    OSStreams[2] = sers;
    OSStreams[3] = log;
    IOinit(kernelLog);

    info(Init,"Mouse initialization");
    mouse.init();
    idt.addInt(0x2c, mouseint);
    pic.activate(Pic::MOUSE);

    info(Init,"Keyboard initialization");
    idt.addInt(0x21,keyboard); // register keyboard interrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.




#ifdef UNITTEST
    unittest();
    stop;
#endif

    info("64 bits kernel booted!!");
    Workspace::draw();

#define BLA CL_TEST
#define EMUL // comment for LORDI version
#if BLA == TMP_TEST

    /*while (true){
        for(int i = 0 ; i < 1024 ; ++i){
            screen.clear();
            screen.set(i,100,Color24::white);
            screen.send();
        }
        }*/


    cl.init();

    while(true){
        kloop();
        debug("Kbd %x",inb(0x60));
    }

#elif BLA == CL_TEST
    HDD::HDD first(1,true);
    first.init();
    HDD::PartitionTableEntry part1 = first[1];
    printf("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    HDD::Partition pa1(&first,part1);
    HDD::Ext2::FS fs1(&pa1);
    HDD::VFS::FS fs(&fs1);

    cl.init();
    cl.pwd = fs.getRoot();

    while(true){
        kloop();
    }

#elif BLA == USER_TEST
    debug("hey!");
    HDD::HDD* first = new HDD::HDD(1,true);
    first->init();
    //PartitionTableEntry part1 = first[1];
    //fb.printf ("Partition 1 beginning at %8x of size %8x \n",part1.begLBA,part1.size);
    //Partition pa1 (&first,part1);
#ifdef EMUL

    HDD::PartitionTableEntry part1 = (*first)[1];
    HDD::Partition* pa1 = new HDD::Partition(first,part1); // load first partition

#else

    const HDD::PartitionTableEntry *part1 = first->partWithPred([](const HDD::Partition&part){
            u32 UUID[4] = {0xe9be36b1,0x894793a2,0x32db3294,0xa907a74a}; // LORDI
            return HDD::Ext2::CheckUUID(UUID,part);
        });
    if(part1 == nullptr)bsod("Main partition not found");
    HDD::Partition* pa1 = new HDD::Partition(first,*part1);
#endif

    HDD::Ext2::FS* fs = new HDD::Ext2::FS(pa1);

    ProcessInit();

    pageLog = true;

    HDD::File* initf = (*(fs->getRoot()))["init"];
    assert(initf);
    assert(initf->getType() == HDD::FileType::RegularFile);
    HDD::RegularFile* init = static_cast<HDD::RegularFile*>(initf);
    ProcessGroup* pg = new ProcessGroup(1);
    Process* initp = new Process(1,pg);
    Thread* initt = initp->loadFromBytes(init);
    TextWindow* initLog = new TextWindow({0,0},screen.getSize(),Font::def);
    initLog->show();
    Workspace::get(1).addWin(initLog);
    FileDescriptor* fd = new FileDescriptor(initLog);
    //FileDescriptor* fd2 = new FileDescriptor(sers);
    printf("Init process %p\n",initp);
    initp->_fds.push_back(FileDescriptor());
    initp->_fds.push_back(*fd);
    //initp->_fds.push_back(*fd2);
    delete fd;
    //delete fd2;
    schedul.init(initt);
    cl.init();
    cl.pwd = fs->getRoot();
    //while(true){
    //     kloop();
    //}
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
    HDD::VFS::FS fs(&fs1);

    HDD::Ext2::FS fs2(&pa2);
    {
        HDD::VFS::Directory* d = static_cast<HDD::VFS::Directory*>((*fs.getRoot())["mnt"]);
        assert(d != nullptr);
        assert(d->getType() == HDD::FileType::Directory);
        fs.mount(d, &fs2);
    }

    HDD::RamFS::FS fs3;
    {
        u16 mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        fs3.getRoot()->addEntry("hello", 0, 0, mode);

        HDD::File* fhello = (*fs3.getRoot())["hello"];
        HDD::RegularFile* frhello = static_cast<HDD::RegularFile*>(fhello);
        const char* helloString = "Hello ";
        const char* worldString = "World!";
        frhello->writeaddr(6, worldString, 6);
        frhello->writeaddr(0, helloString, 6);

        HDD::VFS::Directory* d = static_cast<HDD::VFS::Directory*>((*fs.getRoot())["tmp"]);
        assert(d != nullptr);
        assert(d->getType() == HDD::FileType::Directory);
        fs.mount(d, &fs3);
    }

    HDD::DevFS fs4;
    {
        HDD::VFS::Directory* d = static_cast<HDD::VFS::Directory*>((*fs.getRoot())["dev"]);
        assert(d != nullptr);
        assert(d->getType() == HDD::FileType::Directory);
        fs4.addHardDrive("sda", &first);
        fs.mount(d, &fs4);
    }


    idt.addInt(0x21,keyboard); // register keyborad interrrupt handler
    pic.activate(Pic::KEYBOARD); // activate keyboard interrruption
    kbd.setKeymap(&azertyKeymap); // activate azerty map.
    //kbd.setKeymap(&dvorakKeymap); // activate dvorak map.

    cl.init();
    cl.pwd = fs.getRoot();
    while(true){
        kloop();
    }

#elif BLA == FAT_TEST
    HDD first(1,true);
    first.init()

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
    Workspace::draw();
}




/**
   @brief This function is called as often as possible by scheduler to update the kernel

   No large code in this function should run at each call. it is better to just make a
   condition check and maybe do something if the condition is met

   @todo Do something in kloop like event handling

 */

void kloop(){
    //debug("Entering kloop");

    // poll keyboard
    Keyboard::KeyCode kc;
    while((kc = kbd.poll()).scanCode.valid){
        Workspace::handleEvent(input::Event(kc));
    }

    // poll mouse
    MouseEvent me;
    while((me = mouse.poll()).isValid()) {
        Workspace::handleEvent(input::Event(me));
    }
    Workspace::handleEvent(input::Event(mouse.pollLast()));

    cl.run();

    Workspace::draw();
}




/**
   @brief This function is called to end the kernel, and may, one day, shutdown the computer
*/
[[noreturn]] void kend(){
    cli;
    fprintf(stdlog,
            "\n\n\x1b[35m\x1b[4mKernel is ready to Shutdown,"
            " please press power button for 5s\x1b[m\n");
    Workspace::draw();
    while(true) stop;
}


/**
   @mainpage Super OS Documentation

   Welcome to the Super OS documentation, WIP, good luck !

   @authors Thibaut Pérami, Théophile Wallez

   Thanks to:
       - Luc Chabassier

 */



extern "C" void __cxa_pure_virtual (){}
void * __dso_handle=0;
extern "C" void  __cxa_atexit(){}
