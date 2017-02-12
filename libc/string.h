#ifndef __SUPOS_STRING_H
#define __SUPOS_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int size_t;

void* memcpy(void* dest,const void* src,size_t length);
void* memset(void* dest,int value,size_t length);

#ifdef __cplusplus
}
#endif

#endif
