#ifndef __SUPOS_STDLIB_H
#define __SUPOS_STDLIB_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    #define EXIT_SUCCESS 0
    #define EXIT_FAILURE 1

    int   brk(void* addr);
    void* sbrk(intptr_t offset);
    void* malloc(size_t size);
    void  free (void* ptr);
    void* realloc(void* ptr, size_t size);
    void __initmalloc();
    void __setbrk(void*addr);
    void _exit(int status);
    void exit(int status);


#ifdef __cplusplus
}
#endif


#endif
