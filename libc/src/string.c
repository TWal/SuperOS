#include "../string.h"

void* memcpy(void*dest,const void* src, size_t size){
    asm volatile(
        "rep movsb":
        : "S"(src),"D"(dest),"c"(size)
        );
    return dest;
}

void* memset(void* dest,int value,size_t length){
    asm volatile(
        "rep stosb":
        : "D"(dest),"a"(value),"c"(length)
        );
    return dest;
}


