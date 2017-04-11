#include"../unistd.h"

pid_t fork(){
    int res;
    asm volatile(
        "mov $57, %%rax;"
        "syscall":"=a"(res): :
        );
    return res;
}
