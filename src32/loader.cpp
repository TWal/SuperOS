#include "../src/utility.h"
#include "multiboot.h"
#include "KArgs.h"
#include "../src/Memory/Paging.h"
#include "Segmentation.h"
#include "../src/IO/FrameBuffer.h"
#include "../src/User/Elf64.h"

/**
   @brief Check if long mode is available on this processor

   Call LMunav if long mode is not available
*/
extern "C" void LMcheck();

/**
   @brief Enable long mode and paging

   We are still in a 32bits mode but we are in the compatibility mode of the long mode
   (i.e the mode designed for 32 bits application running under a 64 bit OS)
   and not anymore in protected 32bits mode.

   @param PML4 The first PML4 that must identity map the first Mega-Bytes.
 */
extern "C" void enableLM(void* PML4);

/**
   @brief Start the kernel.

   @param KernelStartPoint The virtual address of kinit
   @param args The KArgs structure sent to kinit.
   @param rsp The position of the stack for kinit

   The function switch to 64bits mode of long mode,
   switch the the stack from the loader one to the kernel one (cf. \ref stack [Stack Page]), and
   switch calling conventions to 64bits mode.
*/
extern "C" void startKernel(u64 KernelStartPoint,KArgs* args,char* rsp); // virtual adress


/// handler if long mode is not available (currently do a blue screen).
extern "C" [[noreturn]] void LMunav(){
    bsod("This Processor does not support 64-bits : unable to boot");
}

typedef void(*funcp)();

extern "C" {
    extern funcp __init_array_start;
    extern funcp __init_array_end;
    extern u32 loader_code_end;
}

/// C++ global constructors initialization.
void init(){
    funcp *beg = & __init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}

/**
   @brief Push an object on another stack than the current one.

   @param t The object to be pushed
   @param rsp the rsp pointer to this stack

   The function is used by \ref load to fill the physical kernel stack
   while still being on loader stack (cf. \ref stack).

 */

template<typename T>
void push(char*& rsp,T t){
    rsp -= sizeof(T);
    *reinterpret_cast<T*>(rsp) = t;
}

/**
   @brief C entry point of the loader.

   @param mb The multiboot structure given by the bootloader.

   Loads the kernel as explain in \ref loading.

 */

extern "C" void load(multibootInfo * mb){
    init();
    LMcheck(); // checking Long mode available
    setupBasicPaging();
    GDTDescriptor gdt;
    gdt.init();
    enableLM(PML4);
    // loading code
    if(mb->mods_count == 0){
        bsod("No module given to loader : unable to load kernel.");
    }
    assert(mb->mods_addr);
    char* kernelAddr = (char*)mb->mods_addr[0].startAddr;
    char* kernelEnd = (char*)mb->mods_addr[0].endAddr;
    u32 kernelSize = kernelEnd - kernelAddr;
    char * kernelStack = (char*)(((u64(kernelEnd) + 0x1000)/0x1000)*0x1000);
    kprintf("Stack start %p",kernelStack);
    char * kernelrsp = kernelStack + 0X1000;
    char * freeMem = kernelrsp;
    assert(freeMem + 0x1000 < (char*)0x600000 && "Initial identity paging too small");
    int occupAreaSize = 1;
    kprintf("Stack start %p",kernelStack);
    push(kernelrsp,OccupArea{(u32)kernelStack,1});
    kprintf("loader from 1MB to %p\n",&loader_code_end);

    kprintf("kernel from %p to %p of size %d\n",kernelAddr,kernelEnd,kernelSize);


    Elf64::Elf64 kernelFile(kernelAddr,kernelSize);

    kprintf("%d sections and %d prog section\n",kernelFile.shnum, kernelFile.phnum);
    /*for(int i = 0; i < kernelFile.shnum ; ++ i){
        auto sh = kernelFile.getSectionHeader(i);
        const char* type = sh.type == Elf64::SHT_PROGBITS ? "ProgBit" :
          (sh.type == Elf64::SHT_NOBITS ? "noBits" : "Other");

        printf("%s, t : %s, off : %llx, virt : %llx, size : %d\n",sh.getName(),type,sh.offset,sh.addr,sh.size);
    }*/
    //printf("\n");

    for(int i = 0; i < kernelFile.phnum ; ++ i){
        auto ph = kernelFile.getProgramHeader(i);

        //printf("%d, t : %d, off : %p, virt : %llx, size :%d %d\n",i,ph.type,ph.getData(),ph.vaddr,ph.filesz,ph.memsz);
        if(ph.type == Elf64::PT_LOAD){
            createMapping(ph.getData(),ph.vaddr,(ph.filesz + 0x1000-1) / 0x1000);
            push(kernelrsp,OccupArea{u32(ph.getData()),u32((ph.filesz + 0x1000-1) / 0x1000)});
            ++occupAreaSize;
        }

    }
    push(kernelrsp,OccupArea{(u32)PML4,1});
    ++occupAreaSize;
    push(kernelrsp,OccupArea{(u32)PDPs,nbPDPused});
    ++occupAreaSize;
    push(kernelrsp,OccupArea{(u32)PDs,nbPDused});
    ++occupAreaSize;
    push(kernelrsp,OccupArea{(u32)PTs,nbPTused});
    ++occupAreaSize;

    KArgs args;
    args.PML4 = u64(PML4);
    args.occupAreaSize = occupAreaSize;
    args.occupArea = u64(kernelrsp);
    args.stackAddr = u64(kernelStack);
    args.freeAddr = u64(freeMem);
    args.RAMSize = mb->mem_upper * 1024;
    push(kernelrsp,args);

    startKernel(kernelFile.entry,(KArgs*)kernelrsp,kernelrsp);
}

/**
   @page loading Booting Sequence

   @brief Description of the loading process.

   @section load_boot Bootloading

   @subsection load_bios BIOS

   The BIOS loads and initialize all devices : the screen, the keyboard, the hard drive, ...
   The the BIOS loads the \ref MBR of the selected HardDrive (or USB key) and load it at
   a predefined address (0x7c00 I think). Then it jumps to it.

   @subsection load_grub GRUB

   GRUB (or another bootloader respecting multiboot convention) now executes. GRUB will locate
   it partition on the Hard-Drive, read boot/grub/grub.cfg and execute its content.
   Currently it is to launch the loader with multiboot convention with the kernel as a module.

   GRUB have now to do two things : switch the processor to 32 bits protected mode and
   launch the loader, sending it the multiboot informations.

   The loader is loaded by GRUB at 1MB
   (following the mapping given by the elf32 file of the loader)
   and the kernel.elf file at an unspecified address beyond.
   The kernel.elf file is copied raw into the RAM.


   @section load_supos SuperOS

   @subsection load_loader Loader

   The SuperOS Loader with then take control and initialize. It will then performs some operations
   (cf. \ref load) :
       - Disable Interrupts.
       - Check Long mode is active on this machine (\ref LMcheck)
       - Setup a basic 64 bit paging that identity map the first 6Mb of RAM.
       - Setup its own GDT.
       - Enable compatibility mode of long mode.
       - Parse the multiboot structure.
       - Create the kernel stack different of its own (cf. \ref stack).
       - Load The elf 64 file by setting up the paging according to its content.
       - Put the KArgs structure in the kernel stack.
       - Launch the Kernel (\ref kinit) in 64 bits structure on its physical stack.

   @subsection load_kernel Kernel

   The kernel is no executing \ref kinit. The kernel will now do more initialization operations :
       - Create its own GDT
       - Create and setup the interrupts table (IDT) and basic interrupts handlers.
       - Enable Interrupts
       - Create the physical memory bitset to dynamically allocate physical memory.
       - Initialize its paging system (Including the its own virtual stack).
       - Switch from physical (by identity mapping) to virtual stack
       - Remove Identity mapping (except the first megabyte for device mapping).
       - Initialize its heap.
       - Initialize sycall and user mode system (cf. syscall and TSS).
       - Read the hard disk, partition table and file system.
       - Load and run the init process.



 */



/**
   @page stack Stacks

   @brief Description of the different stacks of the OS.

   There are 4 different stacks that appear in this project :
       - Loader Stack.
       - Kernel physical stack
       - Kernel stack
       - User mode stack (one by thread actually)

   The loader stack is simple a 4KB block in the data segment of the loader that
   implement the stack, esp is set to its end just before entering @ref load.

   The kernel physical stack is the first page after the area of memory where kernel.elf is loaded
   by the bootloader. rsp is set to it by @ref startKernel just before entering @ref kinit.

   The kernel stack is the last GB of virtual memory (from -1G to 0). The last page of
   the virtual memory is mapped to the physical kernel stack and other page are added under it by
   Paging::allocStack.

   The user stack is somewhere in user-space, by default starting just under 512G.

 */

