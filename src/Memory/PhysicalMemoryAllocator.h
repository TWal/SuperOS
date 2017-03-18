#ifndef PHYSICALMEMORYALLOCATOR
#define PHYSICALMEMORYALLOCATOR

#include "../utility.h"
#include "../../src32/KArgs.h"

#define VIRT_BITSET -0xC0000000

class PhysicalMemoryAllocator {
public:
    PhysicalMemoryAllocator();

    // the Address phyBitset must be identity mapped.
    // the return value is the number of pages the bitset takes
    u64 init(void*phyBitset,u64 RAMSize,OccupArea * occupArea,u64 occupSize);
    void switchVirt(void* virtBitset){_bitset = (u64*)virtBitset;} // Virtual bitset address
    void* alloc();
    void free(void* page);
    void printfirst(int i) ; // print i*64 first bits
private:
    bool get(u64 i);
    void set(u64 i);
    void unset(u64 i);
    bool get(void* addr);
    void set(void* addr);
    void unset(void* addr);
    u64* _bitset;
    size_t _size;
};

extern PhysicalMemoryAllocator physmemalloc;

#endif

