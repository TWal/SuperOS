#ifndef PAGING_H
#define PAGING_H

#include "../utility.h"
#include "../IO/FrameBuffer.h"

#define PDP_NUM 2 // Number of preloaded PageDirectoryPointer
#define PD_NUM 3 // Number of preloaded PageDirectory
#define PT_NUM 4 // Number of preloaded PageTables


struct PageEntry {
#ifdef SUP_OS_KERNEL
    PageEntry();
#endif
    bool present : 1;
    bool readWrite : 1;
    bool user : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool zero : 1;
    bool isSizeMega : 1; // should be zero in higher level than PDE
    int nothing : 4;
    u64 addr : 40;
    u16 data : 11;
    bool zero2 : 1;
    inline void activeAddr(void* a){
        present = true;
        setAddr(a);
    }
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    inline void setAddr(uptr a){
#ifdef SUP_OS_KERNEL
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
#else
        assert((a & ((1<<12)-1)) == 0);
#endif
        addr = a >> 12;
    }
    inline void* getAddr(){
        return (void*)(addr << 12);
    }
    void setAddr32(void* a);
    void setAddr32(u32 a);
} __attribute__((packed));

static_assert(sizeof(PageEntry) == 8, "PageEntry has the wrong size");


struct PageTable {
#ifdef SUP_OS_KERNEL
    PageTable();
#endif
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
    u16 data : 11;
    bool zero2 : 1;
    inline void activeAddr(void* a){
        present = true;
        setAddr(a);
    }
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    inline void setAddr(uptr a){
#ifdef SUP_OS_KERNEL
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
#else
        assert((a & ((1<<12)-1)) == 0);
#endif
        addr = a >> 12;
    }
    inline void* getAddr(){
        return (void*)(addr << 12);
    }
} __attribute__((packed));

static_assert(sizeof(PageTable) == 8, "PageTable has the wrong size");

#ifdef SUP_OS_LOADER

void setupBasicPaging();
void createMapping(void* phy,u64 virt);
void createMapping(void* phy,u64 virt,int numPg);


extern "C" PageEntry PML4[512] __attribute__((section(".data.pages")));
extern "C" PageEntry PDPs[PDP_NUM][512] __attribute__((section(".data.pages")));
extern "C" PageEntry PDs[PD_NUM][512] __attribute__((section(".data.pages")));
extern "C" PageTable PTs[PT_NUM][512] __attribute__((section(".data.pages")));
extern "C" u32 nbPDPused;
extern "C" u32 nbPDused;
extern "C" u32 nbPTused;

#endif


inline int getPML4index(u64 addr){
    return (addr >> (12+9+9+9)) & ((1 << 9)-1);
}

inline int getPDPindex(u64 addr){
    return (addr >> (12+9+9)) & ((1 << 9)-1);
}

inline int getPDindex(u64 addr){
    return (addr >> (12+9)) & ((1 << 9)-1);
}

inline int getPTindex(u64 addr){
    return (addr >> 12) & ((1 << 9)-1);
}

#ifdef SUP_OS_KERNEL


class Paging {
public:
    Paging();
    void init(PageEntry* PML4);
    void allocStack(void*stackPos,size_t nbPages); // assume stack is less than 4K by now
    void removeIdent();// remove identity mappings
    void createMapping(uptr phy, uptr virt);
    void createMapping(uptr phy, uptr virt,int nbPages);
    void freeMapping(uptr virt,int nbPages = 1);
    void freeMappingAndPhy(uptr virt,int nbPages = 1);
    //void switchUser(PageEntry* UserPDP);
    //int brk(void* paddr);
    //void* sbrk(size_t inc); // i64 ?
private:
    void actTmpPDP (void* PDPphyAddr); // activate temporary PD
    void actTmpPD (void* PDphyAddr); // activate temporary PD
    void actTmpPT (void* PTphyAddr); // activate temporary PT
    void freeTmpPDP ();
    void freeTmpPD ();
    void freeTmpPT ();
    void* getPDPphyu(uptr addr); // unsafe version
    void* getPDPphy(uptr addr); // safe version
    void* getPDphyu(uptr addr); // unsafe version
    void* getPDphy(uptr addr); // safe version
    void* getPTphyu(uptr addr); // unsafe version
    void* getPTphy(uptr addr); // safe version
    void* getphyu(uptr addr); // unsafe version
    //uptr _brk;
    //uptr _truebrk;
};

/*struct MallocHeader {
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

extern Paging paging;

#endif // SUP_OS_KERNEL

#endif // PAGING_H
