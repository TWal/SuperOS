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
    void setAddr(void* a);
    void setAddr(uint a);
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
    void setAddr(void* a);
    void setAddr(uint a);
    void* getAddr();
} __attribute__((packed));

static_assert(sizeof(PageTable) == 4, "PageTable has the wrong size");

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

struct MallocHeader {
    uint size : 30;
    uint flags : 2;
    inline int getSize() {
        return size << 2;
    }
    inline void setSize(int sz) {
        assert((sz & ((1<<2)-1)) == 0);
        size = sz >> 2;
    }
};

static_assert(sizeof(MallocHeader) == 4, "MallocHeader has the wrong size");

void initkmalloc();
void* kmalloc(uint size);
void kfree(void* ptr);

extern "C" PageDirectoryEntry PDElower[1024] __attribute__((section(".data.lower")));
extern "C" PageDirectoryEntry* PDE;
extern "C" PageTable PT[1<<20] __attribute__((section(".data.PT")));

#endif
