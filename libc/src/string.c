#include "../string.h"

void* memcpy(void*dest,const void* src,size_t size){
    asm volatile(
        "rep movsb":
        : "S"(src),"D"(dest),"c"(size)
        );
    return dest;
}
