#ifndef __SUPOS_MALLOC_H
#define __SUPOS_MALLOC_H

#include<stddef.h>
#include<assert.h>

struct MallocHeader {
    size_t size : 61;
    bool parity : 1;
    bool prevFree : 1;
    bool free : 1;
    inline bool seemsValid() const  {return (parity ^ prevFree ^ free);}
    inline void update() {parity =!(prevFree ^ free);}
    inline void setFree(bool f) {free=f;update();}
    inline void setPFree(bool pf) {prevFree=pf;update();}
    inline size_t getSize() const {
        return size << 3;
    }
    inline void setSize(size_t sz) {
        assert((sz & ((1<<3)-1)) == 0);
        size = sz >> 3;
    }
};
static_assert(sizeof(MallocHeader) == 8, "MallocHeader has the wrong size");


void initmalloc();
void* malloc(size_t size);
void free(void* ptr);

#endif
