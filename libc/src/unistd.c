#include"../unistd.h"
#include "../errno.h"

// These implementation are only usermode
// kernel must provide (or not) its own implementation
#ifndef SUP_OS_KERNEL
pid_t fork(){
    int res;
    asm volatile(
        "mov $57, %%rax;"
        "syscall":"=a"(res): :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
size_t read(int fd, void* buf, size_t count){
    int res;
    asm volatile(
        "mov $0, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(fd), "S"(buf), "d"(count) // rdi then rsi then rdx
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
size_t write(int fd, const void* buf, size_t count){
    int res;
    asm volatile(
        "mov $1, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(fd), "S"(buf), "d"(count) // rdi then rsi then rdx
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
#endif
