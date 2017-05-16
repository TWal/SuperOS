#ifndef __SUPOS_STDLIB_H
#define __SUPOS_STDLIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    int   brk(void* addr);
    void* sbrk(intptr_t offset);
    void* malloc(size_t size);
    void  free (void* ptr);
    void __initmalloc();
    void __setbrk(void*addr);
    void _exit(int status);


#ifdef __cplusplus
}
#endif


#endif
