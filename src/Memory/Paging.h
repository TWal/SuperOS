#ifndef PAGING_H
#define PAGING_H

#include "../utility.h"




struct PageEntry {
    bool present : 1;
    bool readWrite : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool zero : 1;
    bool isSizeMega : 1; // should be zero in higher level
    int nothing : 4;
    u64 addr : 40;
    u16 data : 10;
    bool zero2 : 1;
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    inline void setAddr(uptr a){
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
        addr = a >> 12;
    }
    void setAddr32(void* a);
    void setAddr32(u32 a);
} __attribute__((packed));

static_assert(sizeof(PageEntry) == 8, "PageEntry has the wrong size");


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
    u64 addr : 40;
    u16 data : 10;
    bool zero2 : 1;
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    inline void setAddr(uptr a){
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
        addr = a >> 12;
    }
    inline void* getAddr(){
        return (void*)(addr << 12);
    }
} __attribute__((packed));

static_assert(sizeof(PageTable) == 8, "PageTable has the wrong size");

extern "C" void setupBasicPaging();

/*class Paging {
    public:
        Paging();
        int brk(void* paddr);
        void* sbrk(size_t inc); // i64 ?
    private:
        uptr _brk;
        uptr _truebrk;
};

struct MallocHeader {
    size_t size : 30;
    bool prevFree : 1;
    bool free : 1;
    inline size_t getSize() {
        return size << 2;
    }
    inline void setSize(size_t sz) {
        assert((sz & ((1<<2)-1)) == 0);
        size = sz >> 2;
    }
};

static_assert(sizeof(MallocHeader) == 4, "MallocHeader has the wrong size");

void initkmalloc();
void* kmalloc(size_t size);
void kfree(void* ptr);*/

extern "C" PageEntry PML4lower[512] __attribute__((section(".data.pages")));
extern "C" PageEntry PDPElower[512] __attribute__((section(".data.pages")));
extern "C" PageEntry PDElower [512] __attribute__((section(".data.pages")));
extern "C" PageEntry* PML4;
extern "C" PageEntry* PDPE;
extern "C" PageEntry* PDE;
//extern "C" PageTable PT[1<<20] __attribute__((section(".data.PT")));

//extern Paging paging;

#endif
