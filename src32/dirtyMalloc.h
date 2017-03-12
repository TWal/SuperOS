#ifndef DMALLOC_H
#define DMALLOC_H

#include "../src/utility.h"

extern "C" void* malloc (size_t size);

extern "C" void free (void* ptr);

#endif
