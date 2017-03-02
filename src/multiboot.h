#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "utility.h"

struct multibootInfo {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
}__attribute__((packed));

static_assert(sizeof(multibootInfo) == 3*4);

#endif

