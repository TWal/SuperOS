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


class UserMemory;

class Paging {
    friend UserMemory;
public:
    Paging();
    void init(PageEntry* PML4);
    void allocStack(void*stackPos,size_t nbPages); // assume stack is less than 4K by now
    void removeIdent();// remove identity mappings
    void createMapping(uptr phy, uptr virt);
    void createMapping(uptr phy, uptr virt,int nbPages);
    void freeMapping(uptr virt,int nbPages = 1);
    void freeMappingAndPhy(uptr virt,int nbPages = 1);
    void* newUserPDP(); // physical address
    void switchUser(void* usPDP);
    void freeUserPDP(void* usPDP);
    void copyUserPDP(void* dest,void* src); // TODO copy on Write
    void copyPhyPages(void*dest,void* src);

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

    void freePTs(void* PD, bool start);
    void freePDs(void* PDP);
    void TLBflush();
    void copyPages(void* PD, bool start);
    void copyPTs(void* PDdest,void*PDsrc, bool start);
    void copyPDs(void* PDPdest,void*PDPsrc);

};

// fixed emplacement in virtual memory.
PageEntry* const virtTables =  (PageEntry*)-0xC0000000ll;

u64* const bitset =  (u64*)-0xA0000000ll; // physmemalloc

PageEntry* const PML4 = virtTables;
PageEntry* const kernelPDP = virtTables + 1*512; // -512G to 0
PageEntry* const kernelPD = virtTables + 2*512; // -2G to -1G
PageEntry* const stackPD = virtTables + 3*512; // -1G to 0
PageEntry* const pagePD = virtTables + 4*512; // -3G to -2G
PageTable* const pagePT = (PageTable*)virtTables + 5*512; // -3G to -3G + 2M
PageEntry* const userPDP = virtTables + 6*512; // 0 to 512G
PageEntry* const firstPD = virtTables + 7*512; // 0 to 1G
PageTable* const firstPT = (PageTable*)virtTables + 8*512; // 0 to 2M
PageTable* const bitsetPT = (PageTable*)virtTables + 9 * 512; // from -2,5G to -2,5G + 2M

#define TMPPDPOFF 0xA
PageEntry* const tmpPDP = virtTables + TMPPDPOFF*512; // temporary manipulation
#define TMPPDOFF 0xB
PageEntry* const tmpPD = virtTables + TMPPDOFF*512; // temporary manipulation
#define TMPPTOFF 0xC
PageTable* const tmpPT = (PageTable*)virtTables + TMPPTOFF * 512; // temporary manipulation

extern Paging paging;

/* Kernel space mappings :
   from -1G to 0 : kernel stack
   from -2G to -1G kernel code and heap.
   from -2G-8K to -2G-4K : tid bitset.
   At -2.5G : physical memory bitset
   At -2.5G - 8K : pageHeap bitset
   At -3G fixed place page address
   from -3G to -3.5G : page Heap
   At -4G temporary space for loading user mode programs
 */

/* User space mappings
   From 0 to 1M : identity mappings to physical devices.
   From 2M to ?? : user code and heap.
   From ?? to 512G users stacks (each thread stack has a 1G space)

 */

#endif // SUP_OS_KERNEL

#endif // PAGING_H
