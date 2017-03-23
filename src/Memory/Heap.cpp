#include "Heap.h"

Heap::Heap() : _virtAddrStart(nullptr),_Brk(0){
}

void Heap::init(void* startAddr){
    _virtAddrStart = (char*)(((uptr)startAddr + 0x1000 -1) / 0x1000 * 0x1000);
}

int Heap::brk(void*addr){
    //printf("recieved address %p\n",addr);
    breakpoint;
    assert((char*)addr > _virtAddrStart);
    uptr dist = (char*)addr - _virtAddrStart;
    if(dist > _Brk){ // if we should allocate new page
        int nbNewPages = (dist - _Brk + 0x1000 -1)/0x1000;
        uptr startAddr = (uptr)_virtAddrStart + _Brk;
        for(int i = 0 ; i < nbNewPages ; ++ i){
            void * phy = physmemalloc.alloc();
            breakpoint;
            paging.createMapping((uptr)phy,startAddr + i * 0x1000);
        }
        _Brk+= nbNewPages * 0x1000;
    }
    else if(dist + 0x1000 < _Brk){ // if we should free some page
        int nbPages = (_Brk - dist) / 0x1000;
        paging.freeMappingAndPhy((uptr)_virtAddrStart + _Brk - nbPages * 0x1000,nbPages);
        _Brk -= nbPages * 0x1000;
    }
    // else do nothing.
    return 0;
}
void* Heap::sbrk(iptr offset){
    char * before = _virtAddrStart + _extBrk;
    int err = brk(before + offset);
    if (!err){
        _extBrk += offset;
        return (void*)before;
    }
    return (void*)(-1);
}

Heap kheap;

int brk(void* addr){
    return kheap.brk(addr);
}

extern "C" void* sbrk(iptr offset){
    return kheap.sbrk(offset);
}

