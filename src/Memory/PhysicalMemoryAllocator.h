#ifndef PHYSICALMEMORYALLOCATOR
#define PHYSICALMEMORYALLOCATOR

#include "../utility.h"

class PhysicalMemoryAllocator {
    public:
        PhysicalMemoryAllocator();
        void* alloc();
        void free(void* page);
    private:
        u32* _bitset;
        u8* _memoryStart;
        size_t _size;
};

extern PhysicalMemoryAllocator physmemalloc;

#endif

