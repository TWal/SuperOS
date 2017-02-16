#ifndef PAGING_H
#define PAGING_H

#include "utility.h"

struct PageDirectoryEntry {
    bool present : 1;
    bool readWrite : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool zero : 1;
    bool isSizeMega : 1;
    int nothing : 4;
    uint PTaddr : 20;
} __attribute__((packed));

static_assert(sizeof(PageDirectoryEntry) == 4, "PageDirectoryEntry has the wrong size");


struct PageTable {
    bool present : 1;
    bool readWrite : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool zero : 1;
    bool global : 1;
    int nothing : 3;
    uint addr : 20;
};

extern "C" void setupBasicPaging() __attribute__((section(".text.lower")));

class Paging {
    public:
        Paging();
        int brk(void* paddr);
        void* sbrk(int inc);
    private:
        uint _brk;
        uint _truebrk;
};

void* kmalloc(uint size);
void kfree(void* ptr);

extern "C" PageDirectoryEntry PDElower[1024] __attribute__((section(".data.lower")));
extern "C" PageDirectoryEntry* PDE;
extern "C" PageTable PT[1<<20] __attribute__((section(".data.PT")));

#endif
