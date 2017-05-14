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
pid_t clone(void(*func)(void),void* stackEnd){
    int res;
    asm volatile(
        "mov $56, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(func), "S"(stackEnd)
        :
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

int open(const char* path, int flags){
    int res;
    asm volatile(
        "mov $2, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(path), "S"(flags)
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
int close(int fd){
    int res;
    asm volatile(
        "mov $3, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(fd)
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
int dup(int oldfd){
    int res;
    asm volatile(
        "mov $32, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(oldfd)
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}

int dup2(int oldfd, int newfd){
    int res;
    asm volatile(
        "mov $33, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(oldfd), "S"(newfd)
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}

pid_t waitpid(pid_t p, int* status){
    int res;
    asm volatile(
        "mov $61, %%rax;"
        "syscall"
        : "=a"(res)
        : "D"(p), "S"(status)
        :
        );
    if (res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
pid_t wait(int* status){
    return waitpid(0,status);
}
#endif
