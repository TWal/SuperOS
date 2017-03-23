#include "Paging.h"
#include "PhysicalMemoryAllocator.h"
#include <new>

// fixed emplacement in virtual memory.

u64* const bitset =  (u64*)-0xA0000000ll;

PageEntry* const virtTables =  (PageEntry*)-0xC0000000ll;
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

// initialization routines

PageEntry::PageEntry() : present(false),readWrite(true),user(false),writeThrough(false),
                         cacheDisable(false),accessed(false),zero(0),isSizeMega(false),
                         nothing(0),addr(0),data(0),zero2(0) {}

PageTable::PageTable() : present(false),readWrite(true),user(false),writeThrough(false),
                         cacheDisable(false),accessed(false),dirty(0),zero(0),global(false),
                         nothing(0),addr(0),data(0),zero2(0) {}

// utility functions
static inline void invlpg(uptr addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}
static inline void invlpg(int addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}
static inline void invlpg(void* addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

// Paging manipulation
Paging::Paging(){
}

void Paging::init(PageEntry * pPML4){ // physical PML4
    assert(pPML4[511].getAddr());
    PageEntry* KPDP = (PageEntry*)pPML4[511].getAddr(); //kernel page directory pointer
    assert(KPDP); // it should be present at kernel booting (setup by the loader)
    PageEntry* PPD = new (physmemalloc.alloc()) PageEntry[512]; // paging page directory
    KPDP[509].activeAddr(PPD);
    invlpg(pagePD);

    PageTable* firstPPT = new(physmemalloc.alloc()) PageTable[512]; // first paging page table
    PPD[0].activeAddr(firstPPT); // activate first page PT
    firstPPT[5].activeAddr(firstPPT); // activate variable pagePT
    invlpg(pagePT);



    // setting up paging tables
    firstPPT[0].activeAddr(pPML4);
    invlpg(PML4);
    firstPPT[1].activeAddr(KPDP); // kernel PDP
    invlpg(kernelPDP);
    assert(KPDP[510].getAddr());
    firstPPT[2].activeAddr(KPDP[510].getAddr()); //kernel PD
    invlpg(kernelPD);

    //stack PD
    PageEntry * stackPDphy = new(physmemalloc.alloc()) PageEntry[512];
    firstPPT[3].activeAddr(stackPDphy); // stackPD
    invlpg(stackPD);
    kernelPDP[511].activeAddr(stackPDphy);

    //pagePD
    firstPPT[4].activeAddr(PPD); // page PD
    invlpg(pagePD);

    //bitset setup
    PageTable* pbitsetPT = new(physmemalloc.alloc()) PageTable[512];
    firstPPT[9].activeAddr(pbitsetPT); // access via bisetPT.
    invlpg(bitsetPT);
    pagePD[256].activeAddr(pbitsetPT); // bitset PageTable activated
    char* pbitset = (char*)physmemalloc.getCurrentAddr();
    for(int i = 0 ; i < physmemalloc.getPageSize() ; ++i){
        bitsetPT[i].activeAddr(pbitset + i * 0x1000);
        invlpg(bitset +i*0x1000);
    }
    physmemalloc.switchVirt(bitset);

    assert(PML4[0].getAddr()); // user PDP is already active
    firstPPT[6].activeAddr(PML4[0].getAddr()); // set user PDP

    assert(userPDP[0].getAddr()); // first PD is already active and identity map the first 4 Mo
    firstPPT[7].activeAddr(userPDP[0].getAddr());

    // mapping of physical RAM from 4K to 1M
    firstPPT[8].activeAddr(new(physmemalloc.alloc()) PageTable[512]);
    for(int i = 1 ; i < 256 ; ++ i){ // RAM mapping to devices
        firstPT[i].activeAddr((char*)nullptr + 0x1000 * i);
        firstPT[i].writeThrough = true ; // only for device mapping RAM
    }
    // we don't activate firstPT i.e removing identity map of RAM begining now.

    // from now all constant address like PML4 or kernelPDP are valid.
    firstPPT[10].writeThrough = true;
    firstPPT[11].writeThrough = true;
    firstPPT[12].writeThrough = true;


}

void Paging::allocStack(void*stackPos,size_t nbPages){
    assert(nbPages < 512); // TODO support more than 2M of stack
    stackPD[511].activeAddr(new(physmemalloc.alloc()) PageTable[512]);
    actTmpPT(stackPD[511].getAddr());

    tmpPT[511].activeAddr(stackPos); // getBack current stack

    for(size_t i = 1 ; i < nbPages ; ++i){
        tmpPT[511 -i].activeAddr(physmemalloc.alloc());
    }
    // we can now use the high space stack

    freeTmpPT();
}
void Paging::removeIdent(){
    firstPD[0].activeAddr(pagePT[8].getAddr());
    firstPD[0].isSizeMega = false;
    firstPD[1].present = false;
    invlpg(0x000000);
    invlpg(0x200000);
}

void Paging::actTmpPDP (void* PDPphyAddr){
    assert(!pagePT[TMPPDPOFF].present);
    pagePT[TMPPDPOFF].activeAddr(PDPphyAddr);
    invlpg((uptr)tmpPDP);
}
void Paging::actTmpPD (void* PDphyAddr){
    assert(!pagePT[TMPPDOFF].present);
    pagePT[TMPPDOFF].activeAddr(PDphyAddr);
    invlpg((uptr)tmpPD);
}
void Paging::actTmpPT (void* PTphyAddr){
    assert(!pagePT[TMPPTOFF].present);
    pagePT[TMPPTOFF].activeAddr(PTphyAddr);
    invlpg((uptr)tmpPT);
}
void Paging::freeTmpPDP (){
    pagePT[TMPPDPOFF].present = false;
}
void Paging::freeTmpPD (){
    pagePT[TMPPDOFF].present = false;
}
void Paging::freeTmpPT (){
    pagePT[TMPPTOFF].present = false;
}


void* Paging::getPDPphyu(u64 addr){ // unsafe version
    return PML4[getPML4index(addr)].getAddr();
}

void* Paging::getPDPphy(u64 addr){ // safe version
    auto PDP = getPDPphyu(addr);
    //printf("Accessing PML4 at %d %p\n",getPML4index(addr),PDP);
    //breakpoint;
    if(PDP) return PDP;
    bsod("More than 2 PDPs ?"); // I only want to support 2 PDPs
    return nullptr; // just for warning
}
void* Paging::getPDphyu(u64 addr){ // unsafe version
    actTmpPDP(getPDPphyu(addr));
    void* res = tmpPDP[getPDPindex(addr)].getAddr();
    freeTmpPDP();
    return res;
}

void* Paging::getPDphy(u64 addr){ // safe version
    auto PDP = getPDPphy(addr);
    actTmpPDP(PDP);

    auto PD = tmpPDP[getPDPindex(addr)].getAddr();
    //printf("Accessing PDP at %d %p\n",getPDPindex(addr),PD);
    if(PD){
        freeTmpPDP();
        return PD;
    }
    PD = physmemalloc.alloc();
    actTmpPD(PD);
    new(tmpPD) PageEntry[512];
    freeTmpPD();
    tmpPDP[getPDPindex(addr)].activeAddr(PD);
    freeTmpPDP();
    return PD;
}

void* Paging::getPTphyu(u64 addr){ // unsafe version
    actTmpPD(getPDphyu(addr));
    void* res = tmpPD[getPDindex(addr)].getAddr();
    freeTmpPD();
    return res;
}
void* Paging::getPTphy(u64 addr){ // safe version
    auto PD = getPDphy(addr);
    actTmpPD(PD);

    auto PT = tmpPD[getPDindex(addr)].getAddr();
    //printf("Accessing PD at %d %p\n",getPDindex(addr),PD);
    if(PT){
        freeTmpPD();
        return PT;
    }
    PT = physmemalloc.alloc();
    actTmpPT(PT);
    new(tmpPT) PageTable[512];
    freeTmpPT();
    tmpPD[getPDindex(addr)].activeAddr(PT);
    freeTmpPD();
    return PT;

  }

void* Paging::getphyu(u64 addr){ // unsafe version
    actTmpPT(getPTphyu(addr));
    void* res = tmpPT[getPTindex(addr)].getAddr();
    freeTmpPT();
    return res;
}
// there is no safe version getphy.



void Paging::createMapping(uptr phy,uptr virt){
    assert(!(phy & ((1<< 12 )-1)) && !(virt & ((1<< 12 )-1))
           && !(phy >> 52));
    //printf("activating %p\n",phy);
    auto PT = getPTphy(virt);
    //printf("PT address get %p\n",PT);
    actTmpPT(PT);
    assert(!tmpPT[getPTindex(virt)].present);
    tmpPT[getPTindex(virt)].activeAddr((void*)phy);
    freeTmpPT();
    invlpg(virt);
}
void Paging::createMapping(uptr phy,uptr virt,int numPg){
    for(int i = 0 ; i < numPg ; ++i){
        createMapping(phy + i * 0x1000,virt+i*0x1000);
    }
}
void Paging::freeMapping(uptr virt,int nbPages){
    for(int i=0 ; i < nbPages ; ++i){
        void * PT = getPTphy(virt+i*0x1000);
        actTmpPT(PT);
        assert(tmpPT[getPTindex(virt+i*0x1000)].present);
        tmpPT[getPTindex(virt+i*0x1000)].present = false;
        freeTmpPT();
    }
}
void Paging::freeMappingAndPhy(uptr virt,int nbPages){
    for(int i = 0 ; i < nbPages ; ++i){
        physmemalloc.free(getphyu(virt));
    }
    freeMapping(virt,nbPages);
}


/*
PageDirectoryEntry PDElower[1024];
PageDirectoryEntry* PDE = (PageDirectoryEntry*)((uint)PDElower + THREEGB);
PageTable PT[1<<20];
const uint MAX_INIT_PAGE = 2;

Paging::Paging() : _brk(THREEGB + (MAX_INIT_PAGE+4)*4*1024*1024), _truebrk(_brk) {
    for(u32 i = (MAX_INIT_PAGE+4); i < 256; ++i) {
        PDE[768+i].setAddr((((uint)&PT[1024*i])-THREEGB));
    }
}

static inline PageTable* getPT(uptr addr) {
    return &PT[(addr-THREEGB)>>12];
}

static inline PageDirectoryEntry* getPDE(uptr addr) {
    return &PDE[addr >> 22];
}


int Paging::brk(void* paddr) {
    uptr addr = (uptr)paddr;
    if(addr > _truebrk) {
        while(addr > _truebrk) {
            if((_truebrk & ((1<<22)-1)) == 0) {
                for(int i = 0; i < 1024; ++i) {
                    getPT(_truebrk)[i].present = false;
                }
                getPDE(_truebrk)->present = true;
            }
            getPT(_truebrk)->setAddr((uptr)physmemalloc.alloc());
            getPT(_truebrk)->present = true;
            invlpg(_truebrk);
            _truebrk += 0x1000;
        }
    } else {
        while(_truebrk - 0x1000 >= addr) {
            _truebrk -= 0x1000;
            physmemalloc.free(getPT(_truebrk)->getAddr());
            getPT(_truebrk)->present = false;
            getPT(_truebrk)->setAddr(nullptr);
            if((_truebrk & ((1<<22)-1)) == 0) {
                getPDE(_truebrk)->present = false;
                getPDE(_truebrk)->setAddr(nullptr);
            }
            invlpg(_truebrk);
        }
    }
    _brk = addr;
    return 0;
}

void* Paging::sbrk(size_t inc) {
    void* res = (void*)_brk;
    brk(((char*)_brk) + inc);
    return res;
}

static MallocHeader* firstHeader;

//static const int FREE = 1;
//static const int PREV_FREE = 2;

void initkmalloc() {
    firstHeader = (MallocHeader*)paging.sbrk(4);
    firstHeader->size = 0;
    firstHeader->prevFree = false;
    firstHeader->free = false;
}

static inline void* headerToPtr(MallocHeader* head) {
    return (void*)((char*)head + sizeof(MallocHeader));
}

static inline MallocHeader* ptrToHeader(void* ptr) {
    return (MallocHeader*)((char*)ptr - sizeof(MallocHeader));
}

static inline MallocHeader* getPrevBoundaryTag(MallocHeader* head) {
    return (MallocHeader*)((char*)head - sizeof(MallocHeader));
}

static inline MallocHeader* getNextBoundaryTag(MallocHeader* head) {
    return (MallocHeader*)((char*)head + head->getSize());
}

static inline MallocHeader* nextHeader(MallocHeader* head) {
    return (MallocHeader*)((char*)head + head->getSize() + sizeof(MallocHeader));
}

static inline size_t align4(size_t n) {
    return n + 4 - (n%4);
}

void* kmalloc(size_t size) {
    size = align4(size);
    MallocHeader* head = firstHeader;
    while(true) {
        //if we are at the end of the linked list
        if(head->size == 0) {
            //allocate enough space
            assert(headerToPtr(head) == paging.sbrk(4+size));
            //setup header
            head->setSize(size);
            head->free = false;
            //setup the next "end of list" header
            MallocHeader* nextHead = nextHeader(head);
            nextHead->size = 0;
            nextHead->prevFree = false;
            nextHead->free = false;
            //return result
            return headerToPtr(head);
            //we didn't do anything with boundary tags because there is nothing to do.
            //(if the previous block is free, head->prevFree is already true)
        }
        //if there is a free block with enough space
        if(head->free && head->getSize() >= size) {
            //change the flag
            head->free = false;
            //if there is enough space to split this block in two
            if(head->getSize() - size >= 2*sizeof(MallocHeader)) {
                MallocHeader* oldNxtHead = nextHeader(head); //only used in the assert
                size_t oldSize = head->getSize();
                //setup the current header
                head->setSize(size);
                MallocHeader* nxtHead = nextHeader(head);
                //setup the header of the next block
                nxtHead->setSize(oldSize - size - sizeof(MallocHeader));
                //it's free and its previous block is allocated
                nxtHead->free = true;
                nxtHead->prevFree = false;
                assert(oldNxtHead == nextHeader(nxtHead));
                //nxtHead is free: copy its boundary tag
                *getNextBoundaryTag(nxtHead) = *nxtHead;
                assert(getNextBoundaryTag(nxtHead) == getPrevBoundaryTag(nextHeader(nxtHead)));
                //oldNxtHead->prevFree is true
                assert(oldNxtHead->prevFree);
            }
            //remove the PREV_FREE flag of next block
            nextHeader(head)->prevFree = false;
            return headerToPtr(head);
        }
        head = nextHeader(head);
    }
}

void kfree(void* ptr) {
    MallocHeader* head = ptrToHeader(ptr);
    //free the block
    assert(!head->free && "This is probably a double free!");
    head->free = true;

    //set the PREV_FREE flag on the next block
    nextHeader(head)->prevFree = true;

    //try to merge with the next block
    MallocHeader* nxtHead = nextHeader(head);
    if(nxtHead->size != 0 && nxtHead->free) {
        MallocHeader* oldNxtNxtHead = nextHeader(nxtHead); //only used in the assert
        //merge
        head->setSize(head->getSize() + nxtHead->getSize() + sizeof(MallocHeader));
        assert(oldNxtNxtHead == nextHeader(head));
    }

    //try to merge with the previous block
    if(head->prevFree) {
        MallocHeader* oldNxtHead = nextHeader(head); //only used in the assert
        //retrieve the previous header using the boundary tag
        MallocHeader* boundTag = getPrevBoundaryTag(head);
        MallocHeader* prevHead = (MallocHeader*)((char*)boundTag - boundTag->getSize());
        assert(nextHeader(prevHead) == head);
        //merge
        prevHead->setSize(prevHead->getSize() + head->getSize() + sizeof(MallocHeader));
        assert(nextHeader(prevHead) == oldNxtHead);
        //copy boundary tag
        *getNextBoundaryTag(prevHead) = *prevHead;
    } else {
        //copy boundary tag
        *getNextBoundaryTag(head) = *head;
    }
    }*/
    Paging paging;


