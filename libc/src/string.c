#include "../string.h"

void* memcpy(void*dest,const void* src, size_t size){
    if(((size_t)dest & 7) ==0 && ((size_t)src & 7) == 0 && ((size_t)size & 7) == 0){
        asm volatile(
            "rep movsq":
            : "S"(src),"D"(dest),"c"(size/8)
            );
    }
    else{
        asm volatile(
            "rep movsb":
            : "S"(src),"D"(dest),"c"(size)
            );
    }
    return dest;
}

void* memset(void* dest,int value,size_t length){
    asm volatile(
        "rep stosb":
        : "D"(dest),"a"(value),"c"(length)
        );
    return dest;
}

#if 0
int memcmp ( const void * ptr1, const void * ptr2, size_t num ){
    return __builtin_memcmp(ptr1,ptr2,num);
}

const void * memchr ( const void * ptr, int value, size_t num ){
    return __builtin_memchr(ptr,value,num);
}

char * strcat ( char * destination, const char * source ){
    return __builtin_strcat(destination,source);
}

const char * strchr ( const char * str, int character ){
    return __builtin_strchr(str,character);
}
#endif

int strcmp(const char* s1, const char* s2) {
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    size_t l = (l1 < l2) ? l1 : l2;
    return strncmp(s1, s2, l+1);
}

int strncmp(const char* s1, const char* s2, size_t n) {
    size_t res;
    asm volatile(
        "repe cmpsb" : "=c"(res)
        : "D"(s1), "S"(s2), "c"(n)
    );
    if(s1[res] == s2[res]) return 0;
    if(s1[res] >  s2[res]) return 1;
    return -1;
}

#if 0
char * strcpy ( char * destination, const char * source ){
    return __builtin_strcpy(destination,source);
}
#endif

char* strncpy (char* dest, const char* src, size_t n) {
    //return __builtin_strncpy(dest, src, n);
    size_t i;
    for(i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    for(; i < n; ++i) {
        dest[i] = '\0';
    }
    return dest;
}

#if 0
size_t strcspn ( const char * str1, const char * str2 ){
    return __builtin_strcspn(str1,str2);
}
#endif

size_t strlen(const char* s) {
    size_t res;
    asm volatile(
        "repne scasb" : "=c"(res)
        : "D"(s), "a"(0), "c"(-1ll)
    );
    return -(res+1)-1;
}

