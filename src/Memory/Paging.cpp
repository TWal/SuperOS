#include "Paging.h"
#include "PhysicalMemoryAllocator.h"
#include "PageHeap.h"
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
    pPML4[511].user = false; //global lock of negative adresses.
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
    for(size_t i = 0 ; i < physmemalloc.getPageSize() ; ++i){
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
        firstPT[i].user = false;
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

bool pageLog = false;

void Paging::actTmpPDP (uptr PDPphyAddr){
    //fprintf(stderr,"Activating TmpPDP at %p\n",PDPphyAddr);
    assert(!pagePT[TMPPDPOFF].present);
    pagePT[TMPPDPOFF].activeAddr(PDPphyAddr);
    invlpg(tmpPDP);
}
void Paging::actTmpPD (uptr PDphyAddr){
    //fprintf(stderr,"Activating TmpPD at %p\n",PDphyAddr);
    assert(!pagePT[TMPPDOFF].present);
    pagePT[TMPPDOFF].activeAddr(PDphyAddr);
    invlpg(tmpPD);
}
void Paging::actTmpPT (uptr PTphyAddr){
    // fprintf(stderr,"Activating TmpPT at %p\n",PTphyAddr);
    assert(!pagePT[TMPPTOFF].present);
    pagePT[TMPPTOFF].activeAddr(PTphyAddr);
    invlpg(tmpPT);
}
void Paging::freeTmpPDP (){
    // fprintf(stderr,"DeActivating TmpPDP\n");
    pagePT[TMPPDPOFF].present = false;
    invlpg(tmpPDP);
}
void Paging::freeTmpPD (){
    // fprintf(stderr,"DeActivating TmpPD\n");
    pagePT[TMPPDOFF].present = false;
    invlpg(tmpPD);
}
void Paging::freeTmpPT (){
    //fprintf(stderr,"DeActivating TmpPT\n");
    pagePT[TMPPTOFF].present = false;
    invlpg(tmpPT);
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
    if(tmpPDP[getPDPindex(addr)].present){
        assert(PD);
        freeTmpPDP();
        return PD;
    }
    PD = physmemalloc.alloc();
    actTmpPD(PD);
    new(tmpPD) PageEntry[512];
    assert(!tmpPD[2].present)
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
    if(tmpPD[getPDindex(addr)].present){
        assert(PT);
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
    //if(pageLog) fprintf(stderr,"mapping %p to %p\n",virt,phy);
    if(phy & ((1<< 12 )-1)) {
        bsod("Create mapping : physical address is not page-aligned : %p",phy);
    }
    if((uptr)virt & ((1<< 12 )-1)){
        bsod("Create mapping : virtual address is not page-aligned : %p",virt);
    }
    if(phy >> 52) {
        bsod("Create mapping : physical address is too large (more than 52 bits)",phy);
    }
    uptr PT = getPTphy(virt);
    actTmpPT(PT);
    if(tmpPT[getPTindex(virt)].present){
        uptr paddr = tmpPT[getPTindex(virt)].getAddr();
        freeTmpPT();
        bsod("The virtual page %p was already mapped to %p",virt,paddr);
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
    for(uint i = 0 ; i < numPg ; ++i){
        createMapping(phy + i * 0x1000,(u8*)virt+i*0x1000,wt);
    }
}


void Paging::freeMapping(void* virt,int nbPages){
    if(pageLog) debug(Pagingl,"free mapping %p to %d",virt,nbPages);
    for(int i=0 ; i < nbPages ; ++i){
        uptr PT = getPTphy((u8*)virt+i*0x1000);
        actTmpPT(PT);
        assert(tmpPT[getPTindex((u8*)virt+i*0x1000)].present);
        tmpPT[getPTindex((u8*)virt+i*0x1000)].present = false;
        freeTmpPT();
    }
}

void Paging::createMapping2M(uptr phy, void* virt, bool wt){
    if(pageLog) debug(Pagingl,"mapping 2M %p to %p",virt,phy);
    if(phy & ((1<< 20 )-1)) {
        bsod("Create mapping 2M : physical address is not 2M-page-aligned : %p",phy);
    }
    if((uptr)virt & ((1<< 20 )-1)){
        bsod("Create mapping : virtual address is not 2M-page-aligned : %p",virt);
    }
    if(phy >> 52) {
        bsod("Create mapping : physical address is too large (more than 52 bits)",phy);
    }
    uptr PD = getPDphy(virt);
    //printf("PT address get %p\n",PT);
    actTmpPD(PD);
    if(tmpPD[getPDindex(virt)].present){
        bsod("The virtual 2M page %p was already mapped to something",virt);
    }
    tmpPD[getPDindex(virt)].activeAddr(phy);
    tmpPD[getPDindex(virt)].writeThrough = wt;
    tmpPD[getPDindex(virt)].isSizeMega = true;
    if(iptr(virt) < 0){ // if we are in kernel space
        //tmpPD[getPDindex(virt)].global = true;
        //tmpPD[getPDindex(virt)].user = false;
    }
    freeTmpPD();
    invlpg(virt);
}

void Paging::createMapping2M(uptr phy,void* virt,uint numPg,bool wt){
    for(uint i = 0 ; i < numPg ; ++i){
        createMapping2M(phy + i * 0x200000,(u8*)virt+i*0x200000,wt);
    }
}

void Paging::freeMapping2M(void* virt,int nbPages){
    if(pageLog) debug(Pagingl,"free 2M mapping %p to %d",virt,nbPages);
    for(int i=0 ; i < nbPages ; ++i){
        uptr PD = getPDphy((u8*)virt+i*0x200000);
        actTmpPD(PD);
        assert(tmpPD[getPDindex((u8*)virt+i*0x200000)].present);
        assert(tmpPD[getPDindex((u8*)virt+i*0x200000)].isSizeMega);
        tmpPD[getPDindex((u8*)virt+i*0x1000)].present = false;
        tmpPD[getPDindex((u8*)virt+i*0x1000)].isSizeMega = false;
        freeTmpPD();
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

void Paging::printPages(uptr PT){
    PageEntry* tPT = pageHeap.alloc<PageEntry>(PT);
    for(int i = 0 ; i < 512 ; ++i){
        if(tPT[i].present){
            if(tPT[i].getAddr() == 0)
                bsod("Entry %d in PT %p was present but set to 0",i,PT);
            info(Pagingl, "\t\t\t\tPage : %d : %p",i,tPT[i].getAddr());
        }
    }
    pageHeap.free(tPT);
}

void Paging::printPTs(uptr PD){
    PageEntry* tPD = pageHeap.alloc<PageEntry>(PD);
    for(int i = 0 ; i < 512 ; ++i){
        if(tPD[i].present){
            if(tPD[i].getAddr() == 0)
                bsod("Entry %d in PD %p was present but set to 0",i,PD);
            if(tPD[i].isSizeMega){
                info(Pagingl, "\t\t\tPT : %d : %p, 2M page",i,tPD[i].getAddr());
            }
            else{
                info(Pagingl, "\t\t\tPT : %d : %p",i,tPD[i].getAddr());
                printPages(tPD[i].getAddr());
            }
        }
    }
    pageHeap.free(tPD);
}

void Paging::printPDs(uptr PDP){
    PageEntry* tPDP = pageHeap.alloc<PageEntry>(PDP);
    for(int i = 0 ; i < 512 ; ++i){
        if(tPDP[i].present){
            if(tPDP[i].getAddr() == 0)
                bsod("Entry %d in PDP %p was present but set to 0",i,PDP);
            info(Pagingl, "\t\tPD : %d : %p", i, tPDP[i].getAddr());
            printPTs(tPDP[i].getAddr());
        }
    }
    pageHeap.free(tPDP);
}

void Paging::print(){
    if(serLvls[Pagingl] < Info) return;
    info(Pagingl,"PML4 : %p",pagePT[0].getAddr());
    info(Pagingl,"\tUserPDP : %p",PML4[0].getAddr());
    printPDs(PML4[0].getAddr());
    info(Pagingl,"\tKernelPDP : %p",PML4[511].getAddr());
    printPDs(PML4[511].getAddr());
}
