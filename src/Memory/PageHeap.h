#ifndef PAGEHEAP_H
#define PAGEHEAP_H

#include"../utility.h"
#include"../Bitset.h"




class PageHeap{
    Bitset _bitset;
    void* ialloc(u64 phy);
public :
    PageHeap();
    void init();
    template<typename T>
    T* alloc(u64 phy){
        return reinterpret_cast<T*>(ialloc(phy));
    }
    void free(void* virt);

};

extern PageHeap pageHeap;

#endif
