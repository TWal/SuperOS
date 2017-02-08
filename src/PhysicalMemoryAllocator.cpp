#include "PhysicalMemoryAllocator.h"

#include "globals.h"

extern "C" void* kernel_code_end;

PhysicalMemoryAllocator::PhysicalMemoryAllocator() :
    _bitset((uint*)((char*)kernel_code_end + 0xC0000000)), _memoryStart(nullptr), _size(0) {

    uint lastsize = 1;
    while(_size != lastsize) {
        lastsize = _size;
        _size = ((uint)(kernel_code_end)+_size-0x00100000)/(4*1024*8) - multiboot.mem_upper / (4*8);
    }

    _memoryStart = (char*)kernel_code_end + _size;
}

void* PhysicalMemoryAllocator::alloc() {
    uint i;
    uchar pos = 0;
    for(i = 0; i < _size; ++i) {
        if(_bitset[i] != 0xFFFFFFFF) {
            if(((_bitset[i]>>pos) & 0xFFFF) == 0xFFFF) pos += 16;
            if(((_bitset[i]>>pos) & 0xFF) == 0xFF) pos += 8;
            if(((_bitset[i]>>pos) & 0xF) == 0xF) pos += 4;
            if(((_bitset[i]>>pos) & 0x3) == 0x3) pos += 2;
            if(((_bitset[i]>>pos) & 0x1) == 0x1) pos += 1;
            assert((_bitset[i] & (1<<pos)) == (uint)0);
            assert((_bitset[i] & ((1<<pos)-1)) == (uint)((1<<pos)-1));
            break;
        }
    }

    _bitset[i] |= (1<<pos);
    return _memoryStart + ((32*i+pos)<<12); //4kb page = 2^12 byte
}

void PhysicalMemoryAllocator::free(void* page) {
    uint index = (uint)((char*)page - _memoryStart);
    assert((index&((1<<12)-1)) == 0);
    index >>= 12;
    uint i = index / 32;
    uchar pos = index % 32;
    assert((_bitset[i] & (1 << pos)) != 0);
    _bitset[i] &= ~(1 << pos);
}

