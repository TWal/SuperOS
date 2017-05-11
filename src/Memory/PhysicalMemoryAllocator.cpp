#include "PhysicalMemoryAllocator.h"
#include "../../src32/KArgs.h"
#include <stdio.h>
#include "../log.h"

extern bool pageLog;
extern "C" void* kernel_code_end;

PhysicalMemoryAllocator::PhysicalMemoryAllocator() : _bitset(nullptr,0){
}

void PhysicalMemoryAllocator::init(void*phyBitset,u64 RAMSize,OccupArea * occupArea,u64 occupSize){
    assert(((u64)phyBitset & (0x1000 -1)) == 0);
    _bitset.init(phyBitset,RAMSize/0x1000);
    _bitset.fill();
    info(PhyMem,"Initializing physical memory with %u MB of RAM",RAMSize/1024/1024);

    debug(PhyMem,"Bitset pageSize %lld",getPageSize());
    for(u64 i = 0 ; i < getPageSize() ; ++i){ // allocate itself
        unset(((uptr)_bitset.getAddr()) + 0x1000 * i);
    }

    for(u64 i = 0 ; i < occupSize ; ++i){
        debug(PhyMem,"Physical allocation of 0x%p with %d",
                occupArea[i].addr,occupArea[i].nbPages);
        for(u64 j = 0 ; j < occupArea[i].nbPages ; ++j){
            unset(occupArea[i].addr + j * 0x1000);
        }
    }
}

bool PhysicalMemoryAllocator::get(uptr addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    return _bitset.get(((u64)addr - 0x100000)/ 0x1000);
}
void PhysicalMemoryAllocator::set(uptr addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    _bitset.set(((u64)addr - 0x100000)/ 0x1000);
}
void PhysicalMemoryAllocator::unset(uptr addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    _bitset.unset(((u64)addr - 0x100000)/ 0x1000);
}

void PhysicalMemoryAllocator::printfirst(int nb){
    for(int j = 0 ; j < nb ; ++j){
        printf("%d",(int)_bitset[j]);
    }
    printf("\n");
}

uptr PhysicalMemoryAllocator::alloc() {
    size_t bsf = _bitset.bsf();
    assert(bsf != (size_t)-1 && "Kernel is out of physical memory");

    _bitset[bsf] = false;
    if(pageLog) debug(PhyMem,"allocating %p",0x100000 + (bsf<<12));
    return 0x100000 + (bsf<<12); //4kb page = 2^12 byte
}

void PhysicalMemoryAllocator::free(uptr page) {
    if(pageLog) debug(PhyMem,"freeing %p",page);
    u64 index = page - 0x100000;
    assert((index&((1<<12)-1)) == 0);
    index >>= 12;

    if(_bitset[index]){
        bsod("Trying to free physical page %p which is already free",page);
    }
    _bitset[index] = true;
}

uptr PhysicalMemoryAllocator::alloc2M(){
    size_t usr = _bitset.usr();
    assert(usr != (size_t)-1 and "Kernel is out of physical memory");
    usr = alignup(usr+1,256);
    if(usr%512 == 0) usr += 256; // hack to handle initial offset of 1M.
    assert(usr + 512 <=  _bitset.size() and "Kernel out of 2M pages");
    if(pageLog) debug(PhyMem,"Allocating 2M %p",0x100000 + (usr<<12));
    _bitset.unset(usr,512);
    return 0x100000 + (usr << 12);
}

void PhysicalMemoryAllocator::free2M(uptr page){
    if(pageLog) debug(PhyMem,"freeing 2M %p",page);
    u64 index = page - 0x100000;
    assert((index&((1<<12)-1)) == 0);
    index >>= 12;
    _bitset.set(index,512);
}

PhysicalMemoryAllocator physmemalloc;
