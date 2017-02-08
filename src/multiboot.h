#ifndef MULTIBOOT_H
#define MULTIBOOT_H

struct multibootInfo {
    int flags;
    uint mem_lower;
    uint mem_upper;
}__attribute__((packed));

static_assert(sizeof(multibootInfo) == 3*4);

#endif

