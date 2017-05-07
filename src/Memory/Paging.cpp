#include "Paging.h"
#include "PhysicalMemoryAllocator.h"
#include <new>
#include <stdio.h>
#include "../log.h"

PageEntry* const Paging::virtTables =  (PageEntry*)-0xC0000000ll;
u64* const Paging::bitset =  (u64*)-0xA0000000ll; // physmemalloc

PageEntry* const Paging::PML4 = virtTables;
PageEntry* const Paging::kernelPDP = virtTables + 1*512; // -512G to 0
PageEntry* const Paging::kernelPD = virtTables + 2*512; // -2G to -1G
PageEntry* const Paging::stackPD = virtTables + 3*512; // -1G to 0
PageEntry* const Paging::pagePD = virtTables + 4*512; // -3G to -2G
PageTable* const Paging::pagePT = (PageTable*)virtTables + 5*512; // -3G to -3G + 2M
PageEntry* const Paging::userPDP = virtTables + 6*512; // 0 to 512G
PageEntry* const Paging::firstPD = virtTables + 7*512; // 0 to 1G
PageTable* const Paging::firstPT = (PageTable*)virtTables + 8*512; // 0 to 2M
PageTable* const Paging::bitsetPT = (PageTable*)virtTables + 9 * 512; // from -2,5G to -2,5G + 2M

#define TMPPDPOFF 0xA
PageEntry* const Paging::tmpPDP = virtTables + TMPPDPOFF*512; // temporary manipulation
#define TMPPDOFF 0xB
PageEntry* const Paging::tmpPD = virtTables + TMPPDOFF*512; // temporary manipulation
#define TMPPTOFF 0xC
PageTable* const Paging::tmpPT = (PageTable*)virtTables + TMPPTOFF * 512; // temporary manipulation
// initialization routines

PageEntry::PageEntry() : present(false),readWrite(true),user(true),writeThrough(false),
                         cacheDisable(false),accessed(false),zero(0),isSizeMega(false),
                         nothing(0),addr(0),data(0),zero2(0) {}

PageTable::PageTable() : present(false),readWrite(true),user(true),writeThrough(false),
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
    assert(this == &paging); // singleton.
}






void Paging::init(PageEntry* pPML4){ // physical PML4
    info(Pagingl,"Paging Initializing");
    assert(pPML4[511].getAddr());
    debug(Pagingl,"PML4 physical address : %p",pPML4);
    PageEntry* KPDP = (PageEntry*)pPML4[511].getAddr(); //kernel page directory pointer
    assert(KPDP); // it should be present at kernel booting (setup by the loader)
    PageEntry* PPD = new ((void*)physmemalloc.alloc()) PageEntry[512]; // paging page directory
    KPDP[509].activeAddr(PPD);

    PageTable* firstPPT = new((void*)physmemalloc.alloc()) PageTable[512]; // first paging page table
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
    PageEntry * stackPDphy = new((void*)physmemalloc.alloc()) PageEntry[512];
    firstPPT[3].activeAddr(stackPDphy); // stackPD
    invlpg(stackPD);
    kernelPDP[511].activeAddr(stackPDphy);

    //pagePD
    firstPPT[4].activeAddr(PPD); // page PD
    invlpg(pagePD);

    //bitset setup
    PageTable* pbitsetPT = new((void*)physmemalloc.alloc()) PageTable[512];
    firstPPT[9].activeAddr(pbitsetPT); // access via bisetPT.
    invlpg(bitsetPT);
    pagePD[256].activeAddr(pbitsetPT); // bitset PageTable activated
    char* pbitset = (char*)physmemalloc.getCurrentAddr();
    for(int i = 0 ; i < physmemalloc.getPageSize() ; ++i){
        bitsetPT[i].activeAddr(pbitset + i * 0x1000);
        bitsetPT[i].global = true;
        invlpg(bitset +i*0x1000);
    }
    physmemalloc.switchVirt(bitset);

    // USER space :
    assert(PML4[0].getAddr()); // user PDP is already active
    firstPPT[6].activeAddr(PML4[0].getAddr()); // set user PDP
    invlpg(userPDP);

    assert(userPDP[0].getAddr()); // first PD is already active and identity map the first 4 Mo
    firstPPT[7].activeAddr(userPDP[0].getAddr());
    invlpg(firstPD);

    // mapping of physical RAM from 4K to 1M
    firstPPT[8].activeAddr(new((void*)physmemalloc.alloc()) PageTable[512]);
    invlpg(firstPT);
    for(int i = 1 ; i < 256 ; ++ i){ // RAM mapping to devices
        firstPT[i].activeAddr((char*)nullptr + 0x1000 * i);
        firstPT[i].writeThrough = true ; // only for device mapping RAM
        firstPT[i].global = true; // these are the only user space mapping to be global.
    }
    // we don't activate firstPT i.e removing identity map of RAM begining now.

    // from now all constant address like PML4 or kernelPDP are valid.
    /*firstPPT[10].writeThrough = true;
    firstPPT[11].writeThrough = true;
    firstPPT[12].writeThrough = true;*/

    for(int i = 0 ; i < 10 ; ++i){
        firstPPT[i].global = true;
    }

    TLBflush();
}






void Paging::allocStack(uptr stackPos,size_t nbPages){
    info(Pagingl,"Allocating %lld pages of stack",nbPages);
    assert(nbPages < 512); // TODO support more than 2M of stack
    stackPD[511].activeAddr(new((void*)physmemalloc.alloc()) PageTable[512]);
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
    firstPD[2].present = false;
    invlpg(0x000000);
    invlpg(0x200000);
    invlpg(0x400000);
}

void Paging::actTmpPDP (uptr PDPphyAddr){
    assert(!pagePT[TMPPDPOFF].present);
    pagePT[TMPPDPOFF].activeAddr(PDPphyAddr);
    invlpg(tmpPDP);
}
void Paging::actTmpPD (uptr PDphyAddr){
    assert(!pagePT[TMPPDOFF].present);
    pagePT[TMPPDOFF].activeAddr(PDphyAddr);
    invlpg(tmpPD);
}
void Paging::actTmpPT (uptr PTphyAddr){
    assert(!pagePT[TMPPTOFF].present);
    pagePT[TMPPTOFF].activeAddr(PTphyAddr);
    invlpg(tmpPT);
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







uptr Paging::getPDPphyu(void* addr){ // unsafe version
    return PML4[getPML4index(addr)].getAddr();
}

uptr Paging::getPDPphy(void* addr){ // safe version
    auto PDP = getPDPphyu(addr);
    //printf("Accessing PML4 at %d %p\n",getPML4index(addr),PDP);
    //breakpoint;
    if(PDP) return PDP;
    bsod("More than 2 PDPs ?"); // I only want to support 2 PDPs
}

uptr Paging::getPDphyu(void* addr){ // unsafe version
    actTmpPDP(getPDPphyu(addr));
    uptr res = tmpPDP[getPDPindex(addr)].getAddr();
    freeTmpPDP();
    return res;
}

uptr Paging::getPDphy(void* addr){ // safe version
    uptr PDP = getPDPphy(addr);
    actTmpPDP(PDP);

    uptr PD = tmpPDP[getPDPindex(addr)].getAddr();
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

uptr Paging::getPTphyu(void* addr){ // unsafe version
    actTmpPD(getPDphyu(addr));
    uptr res = tmpPD[getPDindex(addr)].getAddr();
    freeTmpPD();
    return res;
}
uptr Paging::getPTphy(void* addr){ // safe version
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

uptr Paging::getphyu(void* addr){ // unsafe version
    actTmpPT(getPTphyu(addr));
    uptr res = tmpPT[getPTindex(addr)].getAddr();
    freeTmpPT();
    return res;
}
// there is no safe version getphy.





void Paging::createMapping(uptr phy,void* virt,bool wt){
    //debug(Pagingl,"mapping %p to %p",virt,phy);
    if(phy & ((1<< 12 )-1)) {
        bsod("Create mapping : physical address is not page-aligned : %p",phy);
    }
    if((uptr)virt & ((1<< 12 )-1)){
        bsod("Create mapping : virtual address is not page-aligned : %p",phy);
    }
    if(phy >> 52) {
        bsod("Create mapping : physical address is too large (more than 52 bits)",phy);
    }
    uptr PT = getPTphy(virt);
    //printf("PT address get %p\n",PT);
    actTmpPT(PT);
    if(tmpPT[getPTindex(virt)].present){
        bsod("The virtual page %p was already mapped to %p",virt,tmpPT[getPTindex(virt)].getAddr());
    }
    tmpPT[getPTindex(virt)].activeAddr(phy);
    tmpPT[getPTindex(virt)].writeThrough = wt;
    if(iptr(virt) < 0){ // if we are in kernel space
        tmpPT[getPTindex(virt)].global = true;
    }
    freeTmpPT();
    invlpg(virt);
}


void Paging::createMapping(uptr phy,void* virt,uint numPg,bool wt){
    for(int i = 0 ; i < numPg ; ++i){
        createMapping(phy + i * 0x1000,(u8*)virt+i*0x1000,wt);
    }
}


void Paging::freeMapping(void* virt,int nbPages){
    for(int i=0 ; i < nbPages ; ++i){
        uptr PT = getPTphy((u8*)virt+i*0x1000);
        actTmpPT(PT);
        assert(tmpPT[getPTindex((u8*)virt+i*0x1000)].present);
        tmpPT[getPTindex((u8*)virt+i*0x1000)].present = false;
        freeTmpPT();
    }
}


void Paging::freeMappingAndPhy(void* virt,int nbPages){
    for(int i = 0 ; i < nbPages ; ++i){
        physmemalloc.free(getphyu((u8*)virt +i * 0x1000));
    }
    freeMapping(virt,nbPages);
}






void Paging::TLBflush(){
    asm volatile(
        "mov %%cr3,%%rax;" // globally flush the TLB (except the pages marked as global)
        "mov %%rax,%%cr3" : : :
        "%eax"
        );
}

void Paging::switchUser(uptr usPDP){
    if(usPDP == PML4[0].getAddr()) return; // if this is already the active mapping
    //printf("before activating\n");
    //breakpoint;
    PML4[0].activeAddr(usPDP);
    //printf("before TLBflush\n");
    //breakpoint;
    TLBflush();
    //printf("after TLBflush\n");
}

Paging paging;


