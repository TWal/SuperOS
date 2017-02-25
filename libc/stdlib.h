#ifndef __SUPOS_STDLIB_H
#define __SUPOS_STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#ifdef SUP_OS_KERNEL
EXTERN void * malloc(size_t size);
EXTERN void free (void* ptr);
#else
    #error "Hosted stdlib has not been implemented"
#endif


#ifdef __cplusplus
}
#endif

#undef EXTERN


#endif
