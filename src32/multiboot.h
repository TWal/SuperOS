#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "../src/utility.h"

struct multibootInfo {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    void* mods_addr;
}__attribute__((packed));

static_assert(sizeof(multibootInfo) == 7*4);

extern multibootInfo multiboot;

#endif

