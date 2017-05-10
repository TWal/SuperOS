#ifndef PAGING_H
#define PAGING_H

#include "../utility.h"
#include "../IO/FrameBuffer.h"



#define PDP_NUM 2 ///< Number of preloaded PageDirectoryPointer in loader
#define PD_NUM 3 ///< Number of preloaded PageDirectory in loader
#define PT_NUM 4 ///< Number of preloaded PageTables in loader




/**
   @brief Represent an entry in a PD / a PDP / a PML4

   @sa @ref page_page
*/
struct PageEntry {
#ifdef SUP_OS_KERNEL
    PageEntry();
#endif
    bool present : 1; ///< Whether if this is an active entry (if false other fields are ignored)
    bool readWrite : 1; ///< Whether if write is allowed.
    bool user : 1; ///< define user access rights (set = authorized).
    bool writeThrough : 1; ///< If true any write is immediately flushed to the main memory.
    bool cacheDisable : 1; ///< If true cache is not used for this entry.
    bool accessed : 1; ///< Set to one by processor when this entry is accessed.
    bool zero : 1;

    /**
       @brief Enable a large page mode.

       If this flags is set, the rest of hierarchical structure described
       @ref paging "here" is discarded and the @ref PageEntry::addr "address"
       field is used to describe a larger page.

       For example if the processor, when walking pages, find a PD Entry (PDE)
       whose field isSizeMega is set, it will map the large page (2MB) stating
       virtual address corresponding to this PDE (i.e aligned to 2MB) to the
       physical address in @ref PageEntry::addr "addr" which must be aligned on
       2MB or page fault will be raised.

       In theory you can do the same thing for PDPE with 1GB pages but its need
       more CPU configuration and is currently unused in this project.
     */

    bool isSizeMega : 1;
    int nothing : 4;

    /**
       @brief Contains the physical address corresponding to this entry.

       In fact the whole structure with all others fields set to zero
       correspond to the physical pointer. (Use access function and not direct
       access to set this field).

       Thus AMD64 architecture can never handle more than 52 bits of physical
       memory (4PB).

       @todo This field should become private.

       @sa setAddr, getAddr, activeAddr
     */

    u64 addr : 40;
    u16 data : 11; ///< free to use data, not in use currently
    bool zero2 : 1;

    /// Sets the address to a and active the entry (set present)
    inline void activeAddr(void* a){
        present = true;
        setAddr(a);
    }
    /// Sets the address to a and active the entry (set present)
    inline void activeAddr(uptr a){
        present = true;
        setAddr(a);
    }
    /// Sets the address to a, Use this function and not direct variable access.
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    /// Sets the address to a, Use this function and not direct variable access.
    inline void setAddr(uptr a){
#ifdef SUP_OS_KERNEL
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
        //assert(a <= 4ll*1024*1024*1024);
#else
        assert((a & ((1<<12)-1)) == 0);
#endif
        addr = a >> 12;
    }
    /// Returns the currently setup address.
    inline uptr getAddr(){
        return addr << 12;
    }
} __attribute__((packed));

static_assert(sizeof(PageEntry) == 8, "PageEntry has the wrong size");






/**
   @brief Represent an entry in a PT

   An entry in a PT represent directly a physical page of 4KB.

   @sa @ref page_page

   @todo Think to fusion this class to PageEntry.
*/
struct PageTable {
#ifdef SUP_OS_KERNEL
    PageTable();
#endif
    bool present : 1; ///< @copydoc PageEntry::present
    bool readWrite : 1; ///< @copydoc PageEntry::readWrite
    bool user : 1; ///< @copydoc PageEntry::user
    bool writeThrough : 1; ///< @copydoc PageEntry::writeThrough
    bool cacheDisable : 1; ///< @copydoc PageEntry::cacheDisable
    bool accessed : 1; ///< @copydoc PageEntry::accessed
    bool dirty : 1; ///< Set to one by processor when this page is written
    bool zero : 1;
    /**
       @brief If set, the correspond virtual address will not be flushed on global flush.

       If you set up a page with this flag be sure it should never be changed
       by user mode page switching (see Paging::switchUser)

       @sa @ref page_page and Paging::TLBflush.

    */
    bool global : 1;
    int nothing : 3;
    u64 addr : 40; ///< @copydoc PageEntry::addr
    u16 data : 11; ///< @copydoc PageEntry::data
    bool zero2 : 1;
    /// @copydoc PageEntry::activeAddr(void*)
    inline void activeAddr(void* a){
        present = true;
        setAddr(a);
    }
    /// @copydoc PageEntry::activeAddr(uptr)
    inline void activeAddr(uptr a){
        present = true;
        setAddr(a);
    }
    /// @copydoc PageEntry::setAddr(void*)
    inline void setAddr(void* a){
        setAddr((uptr)a);
    }
    /// @copydoc PageEntry::setAddr(uptr)
    inline void setAddr(uptr a){
#ifdef SUP_OS_KERNEL
        assert((a & ((1<<12)-1)) == 0 && a >> 52 == 0);
        //assert(a <= 4ll*1024*1024*1024);
#else
        assert((a & ((1<<12)-1)) == 0);
#endif
        addr = a >> 12;
    }
    /// @copydoc PageEntry::getAddr()
    inline uptr getAddr(){
        return addr << 12;
    }
} __attribute__((packed));

static_assert(sizeof(PageTable) == 8, "PageTable has the wrong size");






#ifdef SUP_OS_LOADER


///  @brief Loader function that initialize loader paging system and identity map the first 6MB
void setupBasicPaging();
///  @brief Loader function that map the virtual address given to the physical one
void createMapping(void* phy,u64 virt);
///  @brief Loader function that map the virtual address given to the physical one for numPg pages.
void createMapping(void* phy,u64 virt,int numPg);

/// See @ref page_loader "Loader Paging"
extern "C" PageEntry PML4[512] __attribute__((section(".data.pages")));
/// See @ref page_loader "Loader Paging"
extern "C" PageEntry PDPs[PDP_NUM][512] __attribute__((section(".data.pages")));
/// See @ref page_loader "Loader Paging"
extern "C" PageEntry PDs[PD_NUM][512] __attribute__((section(".data.pages")));
/// See @ref page_loader "Loader Paging"
extern "C" PageTable PTs[PT_NUM][512] __attribute__((section(".data.pages")));
/// See @ref page_loader "Loader Paging"
extern "C" u32 nbPDPused;
/// See @ref page_loader "Loader Paging"
extern "C" u32 nbPDused;
/// See @ref page_loader "Loader Paging"
extern "C" u32 nbPTused;



/// Loader function, Get the PML4 index corresponding to this virtual address, see @ref page_page.
inline int getPML4index(u64 addr){
    return (addr >> (12+9+9+9)) & ((1 << 9)-1);
}

///  Loader function, Get the PDP index corresponding to this virtual address, see @ref page_page.
inline int getPDPindex(u64 addr){
    return (addr >> (12+9+9)) & ((1 << 9)-1);
}

/// Loader function. Get the PD index corresponding to this virtual address, see @ref page_page.
inline int getPDindex(u64 addr){
    return (addr >> (12+9)) & ((1 << 9)-1);
}

/// Loader function. Get the PT index corresponding to this virtual address, see @ref page_page.
inline int getPTindex(u64 addr){
    return (addr >> 12) & ((1 << 9)-1);
}

#endif



#ifdef SUP_OS_KERNEL
/// Kernel function, Get the PML4 index corresponding to this virtual address, see @ref page_page.
inline int getPML4index(void* addr){
    return ((uptr)addr >> (12+9+9+9)) & ((1 << 9)-1);
}

/// Kernel function, Get the PDP index corresponding to this virtual address, see @ref page_page.
inline int getPDPindex(void* addr){
    return ((uptr)addr >> (12+9+9)) & ((1 << 9)-1);
}

/// Kernel function, Get the PD index corresponding to this virtual address, see @ref page_page.
inline int getPDindex(void* addr){
    return ((uptr)addr >> (12+9)) & ((1 << 9)-1);
}

/// Kernel function, Get the PT index corresponding to this virtual address, see @ref page_page.
inline int getPTindex(void* addr){
    return ((uptr)addr >> 12) & ((1 << 9)-1);
}


class UserMemory;

/**
   @brief This class handle all the paging of the kernel

   It is a singleton instanced by the variable @ref paging.

   @sa @ref page_page
 */
class Paging {
    friend class UserMemory;
public:
    Paging();
    /**
       @brief Initialize kernel paging.
       @param pPML4 The physical address of the PML4

       This function setup all private global addresses like Paging::PML4, ...

       It also switch the physmemalloc bitset to virtual memory.
     */
    void init(PageEntry* pPML4);
    /**
       @brief Allocate virtual stack
       @param stackPos The address of the physical kernel stack which will
       correspond to the first kernel stack page.
       @param nbPages number of page the kernel stack will take.

       The kernel stack size is 4KB * nbPages. Currently kernel triple fault
       on stack overflow.

       @todo Proper handling of stack overflow.

       @sa @ref stack
    */
    void allocStack(uptr stackPos, size_t nbPages);
    /**
       @brief Removes all identity mappings.

       After this function, all addresses in user space are invalid.
       The only valid positive addresses are from 4K to 1M (device RAM mappings).
     */
    void removeIdent();
    /**
       @brief Map the virtual address virt to phy.
       @param virt The virtual address to be mapped.
       @param phy The destination physical address.
       @param wt If the mapping if writeThrough.

       Currently there is no way to manipulate non active page tables.
       To edit user space mapping you should activate it first.
       (see Paging::switchUser and UserMemory::activate).

       This function dynamically allocate (via @ref physmemalloc) all paging
       structure needed to do the target mapping. It does not, however
       allocate the target physical page pointed by phy.

       If the virtual address is already mapped to a phyiscal address, then a
       @ref bsod "Blue Screen" ensue.


     */
    void createMapping(uptr phy, void* virt, bool wt = false);
    /**
       @brief Map nbPages starting from virt to a physical chunk of same size
       starting from phy.

       @param virt The stating virtual address
       @param phy The starting destination physical address.
       @param nbPages The number of pages to be mapped.

       This function is strictly equivalent to nbPages calls to the other
       @ref createMapping(uptr,void*) "createMapping".

     */
    void createMapping(uptr phy, void* virt, uint nbPages, bool wt = false);
    /**
       @brief Remove the the mapping of the chunk starting at virt with nbPages Pages.
       @param virt The starting virtual address
       @param nbPages The number of pages to be removed

       If the mapping does not exists then, there is a @ref bsod "Blue Screen".
     */
    void freeMapping(void* virt, int nbPages = 1);
    /**
       @brief Remove the the mapping of a chunk and free pointed physical memory
       @param virt The starting virtual address
       @param nbPages The number of pages to be removed

       If the mapping does not exists then, there is a @ref bsod "Blue Screen".

       The physical pages pointed by the freed virtual memory are freed by
       PhysicalMemoryAllocator::free.
    */
    void freeMappingAndPhy(void* virt, int nbPages = 1);
    /**
       @brief Setup a valid user memory PDP.

       UB if usPDP is not valid (probably a triple fault).

       Mainly used by UserMemory, it may not be a good idea to use it outside of UserMemory.
     */
    void switchUser(uptr usPDP);
    /**
       @brief Prints current mappings

       @todo Implement it.
    */
    void print();

private:
    void actTmpPDP (uptr PDPphyAddr); ///< Activate temporary PD
    void actTmpPD (uptr PDphyAddr); ///< Activate temporary PD
    void actTmpPT (uptr PTphyAddr); ///< activate temporary PT
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

    /**
       @brief Globally flush the TLB.

       @sa @ref page_TLB and PageTable::global.

     */

    void TLBflush();

    // fixed emplacements in virtual memory.
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

extern bool pageLog;

/**
   @page mappings Virtual mappings

   Because of a need of simplicity, we have limited ourselves to only 2 PDPs
   The first one which define user space i.e positive addresses : The user PDP.
   The last one wich define kernel space i.e negative addresses : The kernel PDP.

   There only one PML4 which active during all the time the kernel runs.
   It is accessible with Paging::PML4.


   @section map_kernel Kernel space mappings :

   The kernel space is all negative addresses. There is only one kernel PDP
   which is setup by the loader and mapped to the address Paging::kernelPDP.
       - From -1G to 0 : kernel stack (Paging::stackPD)
       - From -2G to -1G kernel code and heap (owned by @ref kheap)
         (Paging::kernelPD)
       - From -2G-8K to -2G-4K : tid bitset for Scheduler
       - At -2.25G : Log buffer
       - At -2.5G : physical memory bitset for @ref physmemalloc
       - At -2.5G - 8K : @ref pageHeap bitset
       - At -3G fixed place page address of Paging's private variables
         (Paging::pagePD and Paging::pagePT)
       - From -3G to -3.1G : @ref pageHeap space
       - At -3.5G : kernel TLS, cf. GDTDescriptor.
       - At -4G temporary space for loading user mode programs
       - At -4.5G : RAM video buffer
       - At -5G : VRAM video buffer


   @section map_user User space mappings

   The user space is all positives addresses. The is one user PDP per process
   plus one default user PDP that map only the first MB.

   A user PDP is said to be *valid* if it identity map addresses from 4K to 1M.
   In fact the identity mapping on the first 1MB is defined in one PT,
   corresponding to Paging::firstPT and whose physical address is given by
   @code{.cpp} Paging::firstPD[0].getAddr() @endcode

   In general (i.e other PDPs than the default), there is those areas :
       - From 0 to 1M : identity mappings to physical devices.
       - From 2M to brk : user code and heap.
       - From ?? to 512G users stacks (each thread stack has a 1G space)
 */


/**
   @page page_page Paging

   @brief Description of the paging system in 64 bits architecture and SuperOS.

   %Paging is system for mapping address in an addressing space to another.

   Here, physical address means an address on the physical RAM
   (the component plugged on the motherboard)

   Virtual address means an address as seen by the processor, OS and application.

   We will *try* to enforce the rule virtual addresses are pointer and physical
   are integers.

   The MMU (Memory Management Unit) makes the translation
   from virtual to physical at each memory access.

   The method is called paging because it decribe a mapping with a granularity
   of pages (4K)


   @section page_arch Architecture AMD64.

   @subsection page_format RAM Format of paging system

   In this architecture, the paging is described by four level of tables :
       - Page Table (PT)
       - Page %Directory (PD)
       - Page %Directory Pointer (PDP)
       - Page Map Level 4 (PML4)

   All those table have 512 entry of size 8 bytes and thus take 4K i.e exactly
   one page.

   The PML4 is the starting point there is only one PML4 active at the same time,
   its physical address is in the cr3 register.

   The PT page has a format described by the struct PageTable and the three other
   have a format described by PageEntry.

   A 64 bit virtual pointer is currently divided in 6 area
       - from 48 to 63 : Unused area : see @subpage canon
       - from 36 to 47 : PML4 index : We look in the PML4 at this address to get
         the physical address of the PDP corresponding at out pointer
       - from 30 to 38 : PDP index : PDP[PDP index] -> physical address of PD
       - from 21 to 29 : PD index : PD[PD index] -> physical address of PT
       - from 12 to 20 : PT index : PT[PT index] -> physical address of the
         beginning of the physical page mapped to the original virtual pointer
       - from 0 to 11 : Final offset : we take the base address of
         the page where we are and add this offset to get to the physical address.

   Trough this commplicated process, the MMU convert virtual pointer to physical
   addresses.

   There is a other way of doing that by page of 2MB, see @ref PageEntry::isSizeMega

   If their is an error at any level, the processor throw a Page fault
   @todo Make a doc page about exceptions

   @subsection page_TLB TLB cache

   Looking through four level of tables can take a long time, thus the mappings
   virtual -> physical are stored in a cache, this can be problematic when you
   want to change the physical address associated with a virtual address.
   The action of reading through the RAM page structure is called "page walking".

   We thus want to flush this cache i.e remove the entry from the TLB
   (and ask to do a page walking the next time this virtual address is needed),
   and eventually flush some other memory cache on this address (L1,L2 or L3).

   There are two ways to do that :
       - Locally by using @ref invlpg instruction : invalidate only one page.
       - Globally by writing to cr3 (see @ref Paging::TLBflush "TLBflush") :
         flush all entries of TLB (except the ones marked as
         @ref PageTable::global "global")

   @section page_supos In Super OS

   @subsection page_loader Loader

   The loader statically allocate (in its .data section) one PML4,
   @ref PDP_NUM PDP in @ref PDPs, @ref PD_NUM PD in @ref PDs, and
   @ref PT_NUM PT in @ref PTs. Each time it needs a table it takes one and
   increment the associated counter
   @ref nbPDPused, @ref nbPDused or @ref nbPTused.
   If we exceed the number of needed pages there is a @ref bsod "Blue Screen".

   This is not a problem because the number of tables needed to map the kernel
   in RAM depend only in the file kernel.elf and thus is known at compile time.

   The loader will just identity map the first 6MB and put the kernel at its
   target address : -2GB. This is done in @ref setupBasicPaging.


   @subsection page_kernel Kernel

   In the kernel all the paging is done through the class Paging.

   The virtual space is divided in two space : kernel space, negative addresses
   and user space : positive addresses. The detailled mapping of virtual memory
   is shown on the documentation page @subpage mappings.
   In general all page table allocation are done dynamically, page tables
   allocated for kernel mapping are never freed (the data physical page are
   freed but not the PT,PD,PDP structure).
   The user space page structure is freed only when the process dies and not
   dynamically during its execution (as before, data page are still freed
   dynamically).


   The physical allocation of all page table is done by @ref physmemalloc as
   any other pages.

   The virtual allocation of page table can be done in three different ways.
       - for page used often and *central*, they are statically allocated
         (cf private static vars of Paging)
       - for simple and early dynamic allocation, their statically allocated
         temporary virtual empalcement, @ref Paging::tmpPDP, @ref Paging::tmpPD,
         @ref Paging::tmpPT.
       - for more complex dynamic allocation, the pageHeap object permit to
         allocate page in virtual memory dynamically.
 */


/**
   @page canon Canonical pointer

   The current AMD64 architecture allow virtual addressing on only 48 bits,
   but in order to prevent ugly hacks to use the 16 remaining bits, AMD force
   any pointer used to be "Canonical" i.e to have its higher 16 bits equal to
   the bit 47.

   If this bit is 0 we have positive address else, it is a negative address.

   Any dereferencing of a non-connnical pointer is followed by a General
   Protection Fault.


 */




#endif // SUP_OS_KERNEL

#endif // PAGING_H
