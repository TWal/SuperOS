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
        physmemalloc.free(getphyu(virt +i * 0x1000));
    }
    freeMapping(virt,nbPages);
}

Paging paging;


