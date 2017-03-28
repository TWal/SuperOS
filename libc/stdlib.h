#ifndef __SUPOS_STDLIB_H
#define __SUPOS_STDLIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUP_OS_KERNEL
    int   brk(void* addr);
    void* sbrk(intptr_t offset);
    void* malloc(size_t size);
    void  free (void* ptr);
    void  initmalloc();
#else
    #error "Hosted stdlib has not been implemented"
#endif


#ifdef __cplusplus
}
#endif


#endif
