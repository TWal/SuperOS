
#include "../src/utility.h"
#include "multiboot.h"
#include "KArgs.h"
#include "../src/Memory/Paging.h"
#include "Segmentation.h"
#include "../src/IO/FrameBuffer.h"

extern "C" void enableLM(void* PML4); // physical adress for PML4
extern "C" void* startKernel(void* KernelStartPoint, u16 seg64,KArgs* args); // virtual adress

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
extern "C" void load(multibootInfo * mb){
    init();
    setupBasicPaging();
    GDTDescriptor gdt;
    gdt.init();
    enableLM(PML4lower);
    startKernel(nullptr,EXEC64BITS,nullptr);
}
