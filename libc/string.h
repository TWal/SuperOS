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
    char * strncpy ( char * destination, const char * source, size_t num);
    size_t strcspn ( const char * str1, const char * str2 );
    size_t strlen ( const char * str );


#ifdef __cplusplus
}
#endif

#endif
