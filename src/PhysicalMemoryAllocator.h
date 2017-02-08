#ifndef PHYSICALMEMORYALLOCATOR
#define PHYSICALMEMORYALLOCATOR

#include "utility.h"

class PhysicalMemoryAllocator {
    public:
        PhysicalMemoryAllocator();
        void* alloc();
        void free(void* page);
    private:
        uint* _bitset;
        char* _memoryStart;
        uint _size;
};

#endif

