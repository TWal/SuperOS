#include "dirtyMalloc.h"

#include "../src/utility.h"

extern "C" uchar loader_stack[];

static uint heapoffset = 0;

void* malloc(size_t size){
    heapoffset += size;
    return loader_stack + ((int)heapoffset - size);
}

void free(void* ptr){
    (void)ptr;
}

 
