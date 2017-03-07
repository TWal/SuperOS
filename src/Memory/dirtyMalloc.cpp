#include "dirtyMalloc.h"
#include "Paging.h"

#include "../utility.h"

extern "C" uchar kernel_heapstack[];

static uint heapoffset = 0;

#if 0

void* malloc(size_t size) {
    return kmalloc(size);
}

void free(void* ptr) {
    kfree(ptr);
}

#else

void* malloc(size_t size){
    heapoffset += size;
    return kernel_heapstack + ((int)heapoffset - size);
}

void free(void* ptr){
    (void)ptr;
}

#endif
