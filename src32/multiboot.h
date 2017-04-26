#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "../src/utility.h"

struct modHeader {
    void * startAddr;
    void * endAddr;
    const char * name;
    u32 zero;
};

struct multibootInfo {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    char* cmdline;
    u32 mods_count;
    modHeader* mods_addr;
    u32 grabage[4];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drive_length;
    u32 drive_addr;
    u32 config_table;
    char* boot_loader_name;
    u32 apm_table;
    u32 VBE_control_info;
    u32 VBE_mode_info;
    u32 VBE_mode;
    u32 VBE_interface_seg;
    u32 VBE_interface_off;
    u32 VBE_interface_len;
    enum{MEMORY = 1, BOOT_DEVICE = 2, CMDLINE = 4,
         MODULES = 8, EXEC1 = 16, EXEC2 = 32,
         MMAP = 64, DRIVE = 128, CONFIG = 256,
         BOOTLOADERNAME = 512, APM = 1024,
         GRAPHICS = 2048
    };
    inline bool check(u32 flag){ return flags & flag;}
}__attribute__((packed));


//static_assert(sizeof(multibootInfo) == 7*4);

extern multibootInfo multiboot;

#endif

