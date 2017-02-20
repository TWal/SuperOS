#include "Paging.h"
#include "utility.h"
#include "globals.h"

const uint HF_BEGIN = THREEGB;


PageDirectoryEntry PDElower[1024];
PageDirectoryEntry* PDE = (PageDirectoryEntry*)((uint)PDElower + HF_BEGIN);
PageTable PT[1<<20];
const int MAX_INIT_PAGE = 2;

void PageDirectoryEntry::setAddr(void* a) {
    setAddr((uint)a);
}

void PageDirectoryEntry::setAddr(uint a) {
    assert((a & ((1<<12)-1)) == 0);
    PTaddr = a >> 12;
}

void PageTable::setAddr(void* a) {
    setAddr((uint)a);
}

void PageTable::setAddr(uint a) {
    assert((a & ((1<<12)-1)) == 0);
    addr = a >> 12;
}

void* PageTable::getAddr() {
    return (void*)(addr << 12);
}

void setupBasicPaging() {
    for(int i = 0; i < 1024; ++i) {
        PDElower[i].present = false;
        PDElower[i].readWrite = true;
        PDElower[i].user = false;
        PDElower[i].writeThrough = true;
        PDElower[i].cacheDisable = false;
        PDElower[i].accessed = false;
        PDElower[i].zero = false;
        PDElower[i].isSizeMega = false;
        PDElower[i].nothing = 0;
        PDElower[i].PTaddr = 0;
    }
    //Identity map the first 4Mb
    PDElower[0].present = true;
    PDElower[0].isSizeMega = true;
    PDElower[0].PTaddr = 0;

    //For the higher half kernel
    for(int i = 0; i < MAX_INIT_PAGE; ++i) {
        PDElower[768+i].present = true;
        PDElower[768+i].isSizeMega = true;
        PDElower[768+i].PTaddr = i<<10;
    }
}

Paging::Paging() : _brk(HF_BEGIN + MAX_INIT_PAGE*4*1024*1024), _truebrk(_brk) {
    for(int i = MAX_INIT_PAGE; i < 256; ++i) {
        PDE[768+i].PTaddr = (((uint)&PT[1024*i])-HF_BEGIN) >> 12;
    }
}

static inline PageTable* getPT(uint addr) {
    return &PT[(addr-HF_BEGIN)>>12];
}

static inline PageDirectoryEntry* getPDE(uint addr) {
    return &PDE[addr >> 22];
}

static inline void invlpg(uint addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

int Paging::brk(void* paddr) {
    uint addr = (uint)paddr;
    if(addr > _truebrk) {
        while(addr > _truebrk) {
            if((_truebrk & ((1<<22)-1)) == 0) {
                for(int i = 0; i < 1024; ++i) {
                    getPT(_truebrk)[i].present = false;
                }
                getPDE(_truebrk)->present = true;
            }
            getPT(_truebrk)->setAddr((uint)physmemalloc.alloc());
            getPT(_truebrk)->present = true;
            invlpg(_truebrk);
            _truebrk += 0x1000;
        }
    } else {
        while(_truebrk - 0x1000 >= addr) {
            _truebrk -= 0x1000;
            physmemalloc.free(getPT(_truebrk)->getAddr());
            getPT(_truebrk)->present = false;
            getPT(_truebrk)->setAddr(nullptr);
            if((_truebrk & ((1<<22)-1)) == 0) {
                getPDE(_truebrk)->present = false;
                getPDE(_truebrk)->setAddr(nullptr);
            }
            invlpg(_truebrk);
        }
    }
    _brk = addr;
    return 0;
}

void* Paging::sbrk(int inc) {
    void* res = (void*)_brk;
    brk(((char*)_brk) + inc);
    return res;
}

void* kmalloc(uint size) {
    return paging.sbrk(size);
}

void kfree(void* ptr) {
    (void)ptr;
}


