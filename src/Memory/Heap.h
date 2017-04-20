#ifndef HEAP_H
#define HEAP_H

#include "PhysicalMemoryAllocator.h"
#include "Paging.h"
/**
   @brief This class represent a heap.

   The heap can be the kernel one (@ref kheap) or a user one (Process::heap).

 */
class Heap{
public:
    /// Empty initialization
    Heap();
    /**
       @brief Initialize this heap.

       This heap will start just after startAddr on a 4K-aligned address which
       is returned by the function.
     */
    void* init(void* startAddr);
    /**
       @brief Move the current break address of the heap.

       @param addr 0 or the new address

       @return The start of the heap if addr is 0, 0 in case of success
       and -1 in case of failure.

       @todo Handle failure properly.

     */
    iptr brk(void* addr);

private :
    char* _virtAddrStart; ///< Start of heap.
    uptr _Brk; ///< true Brk : multiple of 4 KB.
};

/// The heap of the kernel.
extern Heap kheap;

/// Get the kernel beginning of heap.
extern "C" void* getBrk();

extern "C" int brk(void* addr); ///<  brk for libk


#endif
