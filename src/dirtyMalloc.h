#ifndef DMALLOC_H
#define DMALLOC_H

#include <string.h>

extern "C" void* malloc (size_t size);

extern "C" void free (void* ptr);

inline void operator delete (void*ptr){free(ptr);}
inline void operator delete[] (void*ptr){free(ptr);}

#endif
