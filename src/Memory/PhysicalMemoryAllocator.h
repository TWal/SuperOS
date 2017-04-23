#ifndef PHYSICALMEMORYALLOCATOR
#define PHYSICALMEMORYALLOCATOR

#include "../utility.h"
#include "../Bitset.h"
#include "../../src32/KArgs.h"

class PhysicalMemoryAllocator {
public:
    PhysicalMemoryAllocator();

    // the Address phyBitset must be identity mapped.
    // the return value is the number of pages the bitset takes
    void init(void*phyBitset,u64 RAMSize,OccupArea * occupArea,u64 occupSize);
    void switchVirt(void* virtBitset){_bitset.switchAddr(virtBitset);} // Virtual bitset address
    uptr alloc();
    void free(uptr page);
    void printfirst(int i) ; // print i*64 first bits
    size_t getPageSize(){return ((_bitset.size()+7) / 8 + 0x1000 -1) / 0x1000;}
    void* getCurrentAddr(){return _bitset.getAddr();};
private:
    //bool get(u64 i);
    //void set(u64 i);
    //void unset(u64 i);
    bool get(uptr addr);
    void set(uptr addr);
    void unset(uptr addr);
    Bitset _bitset;
    //u64* _bitset; // size in number of bits
    //size_t _size;
};

extern PhysicalMemoryAllocator physmemalloc;

#endif

