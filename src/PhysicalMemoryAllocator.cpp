#include "PhysicalMemoryAllocator.h"

#include "globals.h"

extern "C" void* kernel_code_end;

PhysicalMemoryAllocator::PhysicalMemoryAllocator() :
    _bitset((u32*)((u8*)&kernel_code_end + THREEGB)), _memoryStart(nullptr), _size(0) {

    u32 lastsize = 1;
    //There is usually 2 iterations
    while(_size != lastsize) {
        lastsize = _size;
        _size = multiboot.mem_upper / (4*8) - ((u32)(&kernel_code_end)+_size-0x00100000)/(4*1024*8);
    }

    for(u32 i = 0; i < _size; ++i) {
        _bitset[i] = 0;
    }

    u32 memStart = (u32)&kernel_code_end + _size;
    memStart += 0x1000 - (memStart%0x1000);
    //the start of the memory "overlaps" the first 8MB initially, but that shouldn't be a problem.
    //sometimes adding this line may "fix" a bug, but it is probably somewhere else.
    //memStart += 0x1000000;
    _memoryStart = (u8*)memStart;
}

void* PhysicalMemoryAllocator::alloc() {
    u32 i;
    u8 pos = 0;
    for(i = 0; i < _size; ++i) {
        if(_bitset[i] != 0xFFFFFFFF) {
            if(((_bitset[i]>>pos) & 0xFFFF) == 0xFFFF) pos += 16;
            if(((_bitset[i]>>pos) & 0xFF) == 0xFF) pos += 8;
            if(((_bitset[i]>>pos) & 0xF) == 0xF) pos += 4;
            if(((_bitset[i]>>pos) & 0x3) == 0x3) pos += 2;
            if(((_bitset[i]>>pos) & 0x1) == 0x1) pos += 1;
            assert((_bitset[i] & (1<<pos)) == (u32)0);
            assert((_bitset[i] & ((1<<pos)-1)) == (u32)((1<<pos)-1));
            break;
        }
    }

    _bitset[i] |= (1<<pos);
    return _memoryStart + ((32*i+pos)<<12); //4kb page = 2^12 byte
}

void PhysicalMemoryAllocator::free(void* page) {
    u32 index = (u32)((u8*)page - _memoryStart);
    assert((index&((1<<12)-1)) == 0);
    index >>= 12;
    u32 i = index / 32;
    u8 pos = index % 32;
    assert((_bitset[i] & (1 << pos)) != 0);
    _bitset[i] &= ~(1 << pos);
}

