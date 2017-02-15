#ifndef __SUPOS_STRING_H
#define __SUPOS_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned int size_t;

    void* memcpy(void* dest,const void* src,size_t length);
    void* memset(void* dest,int value,size_t length);
    int memcmp ( const void * ptr1, const void * ptr2, size_t num );
    const void * memchr ( const void * ptr, int value, size_t num );
    void * memmove ( void * destination, const void * source, size_t num );
    char * strcat ( char * destination, const char * source );
    const char * strchr ( const char * str, int character );
    int strcmp ( const char * str1, const char * str2 );
    char * strcpy ( char * destination, const char * source );
    size_t strcspn ( const char * str1, const char * str2 );
    size_t strlen ( const char * str );


#define strcpy __builtin_strcpy
#define strcspn __builtin_strcspn
#define strncat __builtin_strncat
#define strncmp __builtin_strncmp
#define strncpy __builtin_strncpy
#define strpbrk __builtin_strpbrk
#define strrchr __builtin_strrchr
#define strspn __builtin_strspn
#define strstr __builtin_strstr

#ifdef __cplusplus
}
#endif

#endif
