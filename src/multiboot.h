#ifndef MULTIBOOT_H
#define MULTIBOOT_H

struct multibootInfo {
    int flags;
    int mem_lower;
    int mem_upper;
}__attribute__((packed));

static_assert(sizeof(multibootInfo) == 3*4);

#endif

