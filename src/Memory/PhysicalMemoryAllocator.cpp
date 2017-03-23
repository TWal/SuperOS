#include "PhysicalMemoryAllocator.h"
#include "../../src32/KArgs.h"
#include <stdio.h>


extern "C" void* kernel_code_end;

PhysicalMemoryAllocator::PhysicalMemoryAllocator() : _bitset(nullptr),_size(0){
}

u64 PhysicalMemoryAllocator::init(void*phyBitset,u64 RAMSize,OccupArea * occupArea,u64 occupSize){
    assert(((u64)phyBitset & (0x1000 -1)) == 0);
    _bitset = reinterpret_cast<u64*>(phyBitset);
    RAMSize /= 0x1000;
    _size = RAMSize;

    u64 bitsetSize = (RAMSize + 63) / 64;
    for(u64 i = 0 ; i < bitsetSize ; ++ i){
        _bitset[i] = -1; // set bits means free
    }
    //printf("\n");
    //printfirst(1);

    u64 bitsetPageSize = (bitsetSize * 8 + 0x1000 -1) / 0x1000;
    for(u64 i = 0 ; i < bitsetPageSize ; ++i){ // allocate itself
        unset(((uchar*)_bitset) + 0x1000 * i);
    }
    //printfirst(1);

    for(u64 i = 0 ; i < occupSize ; ++i){
        printf("allocation 0x%p with %d\n",occupArea[i].addr,occupArea[i].nbPages);
        //printfirst(1);
        //breakpoint;
        for(u64 j = 0 ; j < occupArea[i].nbPages ; ++j){
            unset(reinterpret_cast<char*>(occupArea[i].addr) + j * 0x1000);
        }
    }
    return bitsetPageSize;
}
bool PhysicalMemoryAllocator::get(u64 i){
    assert(i < _size);
    return _bitset[i/64] >> (i%64) & 1;
}
void PhysicalMemoryAllocator::set(u64 i){
    assert(i < _size);
    _bitset[i/64] |= (1 << (i%64));
}
void PhysicalMemoryAllocator::unset(u64 i){
    //printf("unset %d / %d",i,_size);
    // breakpoint;
    assert(i < _size);
    _bitset[i/64] &= ~(1 << (i%64));
}
bool PhysicalMemoryAllocator::get(void* addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    return get(((u64)addr - 0x100000)/ 0x1000);
}
void PhysicalMemoryAllocator::set(void* addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    set(((u64)addr - 0x100000)/ 0x1000);
}
void PhysicalMemoryAllocator::unset(void* addr){
    assert(((u64)addr & (0x1000 -1)) == 0);
    //printf(" unset 0x%p / %d",addr,_size);
    //breakpoint;
    unset(((u64)addr - 0x100000)/ 0x1000);
}

void PhysicalMemoryAllocator::printfirst(int nb){
    for(int i = 0 ; i < nb ; ++i){
        u64 cur = _bitset[i];
        for(int j = 0 ; j < 64 ; ++j){
            printf("%d",cur&1);
            cur >>= 1;
        }
    }
    printf("\n");
}

void* PhysicalMemoryAllocator::alloc() {
    u64 i;
    u8 pos = 0;
    for(i = 0; i < _size; ++i) {
        if(_bitset[i]) {
            pos = __builtin_ctz(_bitset[i]);
            /*if(((_bitset[i]>>pos) & 0xFFFF) == 0xFFFF) pos += 16;
            if(((_bitset[i]>>pos) & 0xFF) == 0xFF) pos += 8;
            if(((_bitset[i]>>pos) & 0xF) == 0xF) pos += 4;
            if(((_bitset[i]>>pos) & 0x3) == 0x3) pos += 2;
            if(((_bitset[i]>>pos) & 0x1) == 0x1) pos += 1;
            assert((_bitset[i] & (1<<pos)) == (u32)0);
            assert((_bitset[i] & ((1<<pos)-1)) == (u32)((1<<pos)-1));*/
            break;
        }
    }
    assert(i < _size && "out of memory");

    _bitset[i] &= ~(1<<pos);
    //printf("allocating %p",(0x100000 + ((64*i+pos)<<12)));
    return (void*)(0x100000 + ((64*i+pos)<<12)); //4kb page = 2^12 byte
}

void PhysicalMemoryAllocator::free(void* page) {
    //printf("freeing %p",page);
    u64 index = (u64)((u8*)page - 0x100000);
    assert((index&((1<<12)-1)) == 0);
    index >>= 12;
    u64 i = index / 64;
    u8 pos = index % 64;
    assert((_bitset[i] & (1 << pos)) != 1);
    _bitset[i] |= (1 << pos);
}

PhysicalMemoryAllocator physmemalloc;
