#ifndef HEAP_H
#define HEAP_H

#include "PhysicalMemoryAllocator.h"
#include "Paging.h"

class Heap{
public:
    Heap(); // must be aligned on page boundaries
    void* init(void* startAddr);
    int brk(void* addr);

private :
    char* _virtAddrStart;
    uptr _Brk; // true Brk : multiple of 4 KB.
};

extern Heap kheap;

extern "C" int brk(void* addr); // brk for libc


#endif
