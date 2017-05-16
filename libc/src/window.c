#include "../window.h"
#include "../errno.h"


// These implementation are only usermode
// kernel must provide (or not) its own implementation
#ifndef SUP_OS_KERNEL

int openwin(vec_t size, vec_t offset, int workspace){
    int res;
    asm volatile(
        "mov $24, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(size), "S"(offset), "d"(workspace) // rdi then rsi then rdx
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}

#endif
