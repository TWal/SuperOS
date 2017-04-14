#ifndef PAGING_H
#define PAGING_H

#include "../utility.h"
#include "../IO/FrameBuffer.h"



#define PDP_NUM 2 ///< Number of preloaded PageDirectoryPointer in loader
#define PD_NUM 3 ///< Number of preloaded PageDirectory in loader
#define PT_NUM 4 ///< Number of preloaded PageTables in loader

/**
   @brief Represent an entry in a PD / a PDP / a PML4
*/
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
    inline void activeAddr(uptr a){
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
    inline uptr getAddr(){
        return addr << 12;
    }
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
    inline void activeAddr(uptr a){
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
    inline uptr getAddr(){
        return addr << 12;
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

#endif



#ifdef SUP_OS_KERNEL
inline int getPML4index(void* addr){
    return ((uptr)addr >> (12+9+9+9)) & ((1 << 9)-1);
}

inline int getPDPindex(void* addr){
    return ((uptr)addr >> (12+9+9)) & ((1 << 9)-1);
}

inline int getPDindex(void* addr){
    return ((uptr)addr >> (12+9)) & ((1 << 9)-1);
}

inline int getPTindex(void* addr){
    return ((uptr)addr >> 12) & ((1 << 9)-1);
}


class UserMemory;

class Paging {
    friend class UserMemory;
public:
    Paging();
    void init(PageEntry* pPML4);
    void allocStack(void*stackPos,size_t nbPages); // assume stack is less than 4K by now
    void removeIdent();// remove identity mappings
    void createMapping(uptr phy, void* virt);
    void createMapping(uptr phy, void* virt,int nbPages);
    void freeMapping(void* virt,int nbPages = 1);
    void freeMappingAndPhy(void* virt,int nbPages = 1);
    void switchUser(uptr usPDP);

private:
    void actTmpPDP (uptr PDPphyAddr); // activate temporary PD
    void actTmpPD (uptr PDphyAddr); // activate temporary PD
    void actTmpPT (uptr PTphyAddr); // activate temporary PT
    void freeTmpPDP ();
    void freeTmpPD ();
    void freeTmpPT ();
    uptr getPDPphyu(void* addr); // unsafe version
    uptr getPDPphy(void* addr); // safe version
    uptr getPDphyu(void* addr); // unsafe version
    uptr getPDphy(void* addr); // safe version
    uptr getPTphyu(void* addr); // unsafe version
    uptr getPTphy(void* addr); // safe version
    uptr getphyu(void* addr); // unsafe version

    void TLBflush();

    // fixed emplacement in virtual memory.
    static PageEntry* const virtTables;
    static u64* const bitset; // physmemalloc

    static PageEntry* const PML4;
    static PageEntry* const kernelPDP; // -512G to 0
    static PageEntry* const kernelPD; // -2G to -1G
    static PageEntry* const stackPD; // -1G to 0
    static PageEntry* const pagePD; // -3G to -2G
    static PageTable* const pagePT; // -3G to -3G + 2M
    static PageEntry* const userPDP; // 0 to 512G
    static PageEntry* const firstPD; // 0 to 1G
    static PageTable* const firstPT; // 0 to 2M
    static PageTable* const bitsetPT; // from -2,5G to -2,5G + 2M

#define TMPPDPOFF 0xA
    static PageEntry* const tmpPDP; // temporary manipulation
#define TMPPDOFF 0xB
    static PageEntry* const tmpPD; // temporary manipulation
#define TMPPTOFF 0xC
    static PageTable* const tmpPT; // temporary manipulation
};



extern Paging paging;

/**
   @page mappings Virtual mappings

   @section map_kernel Kernel space mappings :
       - from -1G to 0 : kernel stack
       - from -2G to -1G kernel code and heap (owned by @ref kheap)
       - from -2G-8K to -2G-4K : tid bitset for Scheduler
       - At -2.5G : physical memory bitset for @ref physmemalloc
       - At -2.5G - 8K : @ref pageHeap bitset
       - At -3G fixed place page address of Paging
       - from -3G to -3.5G : @ref pageHeap space
       - At -4G temporary space for loading user mode programs


   @section map_user User space mappings
       - From 0 to 1M : identity mappings to physical devices.
       - From 2M to brk : user code and heap.
       - From ?? to 512G users stacks (each thread stack has a 1G space)

 */


/**
   @page paging Paging

   @brief Description of the paging system in 64 bits architecture and SuperOS.

   Paging is system for mapping address in an addressing space to another.

   Here, physical address means an address on the physical RAM
   (the component plugged on the motherboard)

   Virtual address means an address as seen by the processor, OS and application.

   We will *try* to enforce the rule virtual addresses are pointer and physical are integers.

   The MMU (Memory Management Unit) makes the translation
   from virtual to physical at each memory access.

   The method is called paging because it decribe a mapping with a granularity of pages (4K)


   @section page_arch Architecture AMD64.

   @subsection page_format RAM Format of paging system

   In this architecture, the paging is described by four level of tables :
       - Page Table (PT)
       - Page %Directory (PD)
       - Page %Directory Pointer (PDP)
       - Page Map Level 4 (PML4)

   All those table have 512 entry of size 8 bytes and thus take 4K i.e exactly one page.

   The PML4 is the starting point there is only one PML4 active at the same time,
   its physical address is in the cr3 register.

   The PT page has a format described by the struct PageTable and the three other have a
   format described by PageEntry.

   A 64 bit virtual pointer is currently divided in 6 area
       - from 48 to 63 : Unused area : all those bit must be the same than the bit 47.
         If this is 0 we have positive address else, it is a negative address.
       - from 36 to 47 : PML4 index : We look in the PML4 at this address to get the physical
         address of the PDP corresponding at out pointer
       - from 30 to 38 : PDP index : PDP[PDP index] -> physical address of PD
       - from 21 to 29 : PD index : PD[PD index] -> physical address of PT
       - from 12 to 20 : PT index : PT[PT index] -> physical address of the
         beginning of the physical page mapped to the original virtual pointer
       - from 0 to 11 : Final offset : we take the base address of
        the page where we are and add this offset to get to the physical address.

   Trough this commplicated process, the MMU convert virtual pointer to physical addresses.

   There is a other way of doing that by page of 2MB, see @ref PageEntry::isSizeMega

   If their is an error at any level, the processor throw a Page fault
   @todo Make a doc page about exceptions

   @subsection page_TLB TLB cache.

   Looking through four level of tables can take a long time, thus the mapping
   virtual -> physical are stored in a cache, this can be problematic when you
   want to change the physical address associated with a virtual address.
   The action of reading through the RAM page structure is called "page walking".

   We thus want to flush this cache i.e remove the entry from the TLB
   (and ask to do a page walking the next time this virtual address is needed),
   and eventually flush some other memory cache on this address (L1,L2 or L3).

   There are two ways to do that :
       - Locally by using @ref invlpg instruction : invalidate only one page.
       - Globally by writing to cr3 (see @ref Paging::TLBflush "TLBflush") : flush all entries of
         TLB (except the ones marked as @ref PageTable::global "global")

   @section page_supos In Super OS

   @subsection page_loader Loader

   The loader statically allocate (in its .data section) one PML4, @ref PDP_NUM PDP in @ref PDPs,
   @ref PD_NUM PD in @ref PDs, and @ref PT_NUM PT in @ref PTs.
   Each time it needs a table it takes one and increment the associated counter
   @ref nbPDPused, @ref nbPDused or @ref nbPTused.
   If we exceed the number of needed pages there is a @ref bsod "Blue Screen".

   This is not a problem because the number of tables needed to map the kernel in RAM depend only
   in the file kernel.elf and thus is known at compile time.

   The loader will just identity map the first 6MB and put the kernel at its target address : -2GB.
   This is done in @ref setupBasicPaging.


   @subsection page_kernel Kernel

   In the kernel all the paging is done through the class Paging.

   The virtual space is divided in two space : kernel space, negative addresses
   and user space : positive addresses. The detailled mapping of virtual memory is shown on
   the documentation page @subpage mappings.
   In general all page table allocation are done dynamically, page tables allocated
   for kernel mapping are never freed (the data physical page are freed but not
   the PT,PD,PDP structure).
   The user space page structure is freed only when the process dies and not dynamically
   during its execution (as before, data page are still freed dynamically).


   The physical allocation of all page table is done by @ref physmemalloc as any other pages.

   The virtual allocation of page table can be done in three different ways.
       - for page used often and *central*, they are statically allocated (cf private
         static vars of Paging)
       - for simple and early dynamic allocation, their statically allocated temporary
         virtual empalcement, @ref tmpPDP, @ref tmpPD, @ref tmpPT.
       - for more complex dynamic allocation, the pageHeap object permit to
         allocate page in virtual memory dynamically.

 */




#endif // SUP_OS_KERNEL

#endif // PAGING_H
