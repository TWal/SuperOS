#include "../src/utility.h"
#include "multiboot.h"
#include "KArgs.h"
#include "../src/Memory/Paging.h"
#include "Segmentation.h"
#include "../src/IO/FrameBuffer.h"
#include "../src/User/Elf64.h"


extern "C" void enableLM(void* PML4); // physical adress for PML4
extern "C" void startKernel(u64 KernelStartPoint,KArgs* args,char* rsp); // virtual adress

typedef void(*funcp)();

extern "C" {
    extern funcp __init_array_start;
    extern funcp __init_array_end;
    extern u32 loader_code_end;
}

void init(){
    funcp *beg = & __init_array_start, *end = & __init_array_end;
    for (funcp*p = beg; p < end; ++p){
        (*p)();
    }
}

template<typename T>
void push(char*& rsp,T t){
    rsp -= sizeof(T);
    *reinterpret_cast<T*>(rsp) = t;
}



extern "C" void load(multibootInfo * mb){
    init();
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
    printf("Stack start %p",kernelStack);
    char * kernelrsp = kernelStack + 0X1000;
    char * freeMem = kernelrsp;
    assert(freeMem + 0x1000 < (char*)0x600000 && "Initial identity paging too small");
    int occupAreaSize = 1;
    printf("Stack start %p",kernelStack);
    push(kernelrsp,OccupArea{(u32)kernelStack,1});
    printf("loader from 1MB to %p\n",&loader_code_end);

    printf("kernel from %p to %p of size %d\n",kernelAddr,kernelEnd,kernelSize);


    Elf64::Elf64 kernelFile(kernelAddr,kernelSize);

    printf("%d sections and %d prog section\n",kernelFile.shnum, kernelFile.phnum);
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
