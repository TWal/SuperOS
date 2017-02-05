#ifndef PAGING_H
#define PAGING_H

struct PageDirectoryEntry {
    bool present : 1;
    bool readWrite : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool zero : 1;
    bool isSizeMega : 1;
    int nothing : 3;
    int PTaddr : 20;
} __attribute__((packed));

static_assert(sizeof(PageDirectoryEntry) == 4, "PageDirectoryEntry has the wrong size");


struct PageTable {
};

extern "C" void setupPaging() __attribute__((section(".text.lower")));
extern "C" PageDirectoryEntry PDElower[1024] __attribute__((section(".data.lower")));
extern "C" PageDirectoryEntry* PDE;

#endif
