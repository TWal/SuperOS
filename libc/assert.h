#ifndef __SUPOS_ASSERT_H
#define __SUPOS_ASSERT_H

#include "stdlib.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif
#define __S(x) #x
#define __S_(x) __S(x)
#define S__LINE__ __S_(__LINE__)

#ifdef SUP_OS_KERNEL
    __attribute__((noreturn)) void bsod(const char* s, ...);

#define assert(cond) {                                                  \
        if(!(cond)) {                                                   \
            bsod("Assertion failed at " __FILE__ ":" S__LINE__ ": " #cond); \
        }                                                               \
    }
#else

    // TODO printf something.
#define assert(cond) {                                                  \
        if(!(cond)) {                                                   \
            printf("Assertion failed at " __FILE__ ":" S__LINE__ ": " #cond); \
            _exit(1);                                                 \
        }                                                               \
    }


#endif

#ifdef __cplusplus
}
#endif


#endif
