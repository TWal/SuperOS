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

int memcmp ( const void * ptr1, const void * ptr2, size_t num ){
    return __builtin_memcmp(ptr1,ptr2,num);
}

const void * memchr ( const void * ptr, int value, size_t num ){
    return __builtin_memchr(ptr,value,num);
}

void * memmove ( void * destination, const void * source, size_t num ){
    return memcpy(destination,source,num);
}

char * strcat ( char * destination, const char * source ){
    return __builtin_strcat(destination,source);
}

const char * strchr ( const char * str, int character ){
    return __builtin_strchr(str,character);
}

int strcmp ( const char * str1, const char * str2 ){
    return __builtin_strcmp(str1,str2);
}

char * strcpy ( char * destination, const char * source ){
    return __builtin_strcpy(destination,source);
}

size_t strcspn ( const char * str1, const char * str2 ){
    return __builtin_strcspn(str1,str2);
}

size_t __attribute__((optimize("O0"))) strlen ( const char * str ){
    return __builtin_strlen(str);
}

