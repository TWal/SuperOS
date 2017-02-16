#include "dirtyMalloc.h"

#include "utility.h"
#include "globals.h"

extern "C" uchar kernel_heapstack[];

static uint heapoffset;

void* malloc(size_t size){
    heapoffset += size;
    return kernel_heapstack + ((int)heapoffset - size);
}

void free(void* ptr){
    (void)ptr;
}
