#ifndef PHYSICALMEMORYALLOCATOR
#define PHYSICALMEMORYALLOCATOR

#include "../utility.h"
#include "../Bitset.h"
#include "../../src32/KArgs.h"


/**
   @brief This singleton handle the physical memory ressources, allocate and unallocate it.

   It uses a internal bitset allocate statically at initialization.
 */
class PhysicalMemoryAllocator {
public:
    PhysicalMemoryAllocator();

    /**
       @brief Initialize the object with a given physical address
       @param phyBitset Physical address where to put the bitset, all memory
       should be free after this point.
       @param RAMSize Size of RAM in byte.
       @param occupArea The address to the table describing occupied area.
       @param occupSize The size of the table describing occupied area.

       All addresses must be physical address identity mapped.
    */
    void init(void*phyBitset, u64 RAMSize, OccupArea* occupArea, u64 occupSize);

    /// Change the address of the bitset.
    void switchVirt(void* virtBitset){_bitset.switchAddr(virtBitset);} // Virtual bitset address

    /// Allocate a 4K physical page.
    uptr alloc();
    /// Free a 4K physical page.
    void free(uptr page);
    /// Allocate a 2M physical page.
    uptr alloc2M();
    /// Free a 2M physical page.
    void free2M(uptr page);

    /// debug @todo delete this
    void printfirst(int i) ; // print i*64 first bits
    /// Returns the number of 4K pages used for the bitset.
    size_t getPageSize(){return ((_bitset.size()+7) / 8 + 0x1000 -1) / 0x1000;}

    /// Returns the current address used for the bitset.
    void* getCurrentAddr(){return _bitset.getAddr();};
private:
    bool get(uptr addr);
    void set(uptr addr);
    void unset(uptr addr);
    Bitset _bitset;
};

extern PhysicalMemoryAllocator physmemalloc;

#endif

