#include "UserMemory.h"
#include "PhysicalMemoryAllocator.h"
#include "Paging.h"
#include "PageHeap.h"

UserMemory::UserMemory(): _PDP((u64)physmemalloc.alloc()){
    paging.actTmpPDP(_PDP);
    new(Paging::tmpPDP) PageEntry[512];
    uptr phyPD = physmemalloc.alloc();
    paging.actTmpPD(phyPD);
    new(Paging::tmpPD) PageEntry[512];

    Paging::tmpPDP[0].activeAddr(phyPD);
    assert(Paging::firstPD[0].getAddr());
    Paging::tmpPD[0].activeAddr(Paging::firstPD[0].getAddr());
    paging.freeTmpPD();
    paging.freeTmpPDP();
}

void UserMemory::activate(){
    paging.switchUser(_PDP);
}

void UserMemory::freePages(uptr PT){
    paging.actTmpPT(PT);
    for(int i =  0 ; i < 512 ; ++i){
        if(Paging::tmpPT[i].present){
            assert(Paging::tmpPT[i].getAddr());
            physmemalloc.free(Paging::tmpPT[i].getAddr());
        }
    }
    paging.freeTmpPT();
}

void UserMemory::freePTs(uptr PD, bool start){
    paging.actTmpPD(PD);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(Paging::tmpPD[i].getAddr()&& Paging::tmpPD[i].present){
            freePages(Paging::tmpPD[i].getAddr());
            physmemalloc.free(Paging::tmpPD[i].getAddr());
            if(start){
                //in this case we are on Paging::firstPD : it will remain after free, we must clean it.
                Paging::tmpPD[i].present = false;
            }
        }
    }
    paging.freeTmpPD();
}

void UserMemory::freePDs(uptr PDP){
    paging.actTmpPDP(PDP);
    freePTs(Paging::tmpPDP[0].getAddr(),true);
    for(int i = 1 ; i < 512 ; ++i){
        uptr PD;
        if( (PD = Paging::tmpPDP[i].getAddr()) && Paging::tmpPDP[i].present ){
            freePTs(PD,false);
            physmemalloc.free(PD);
            Paging::tmpPDP[i].present = false;
        }
    }
    paging.freeTmpPDP();
}
void UserMemory::clear(){
    assert(_PDP != Paging::pagePT[6].getAddr()); // ensure we do not delete the kernel userPDP.
    if(_PDP == Paging::PML4[0].getAddr()){ // if this mapping is active return to the default mapping
        paging.switchUser(Paging::pagePT[6].getAddr());
    }
    freePDs(_PDP);
}

UserMemory::~UserMemory(){
    clear();
    // Deleting first PD
    paging.actTmpPDP(_PDP);
    physmemalloc.free(Paging::tmpPDP[0].getAddr());
    paging.freeTmpPDP();
    // Deleting PDP.
    physmemalloc.free(_PDP);
}

void UserMemory::copyPages(uptr PTdest,uptr PTsrc){
    assert(PTdest && PTsrc);
    PageTable * src = pageHeap.alloc<PageTable>(PTsrc);
    PageTable * dest = pageHeap.alloc<PageTable>(PTdest);
    for(int i = 0 ; i < 512 ; ++i){
        if(src[i].present){
            dest[i] = src[i];
            uptr newp = physmemalloc.alloc(); // TODO copy on demand
            uptr oldp = src[i].getAddr();
            void* psrc = pageHeap.alloc<void>(oldp);
            void* pdest = pageHeap.alloc<void>(newp);
            memcpy(pdest,psrc,0x1000);
            pageHeap.free(psrc);
            pageHeap.free(pdest);
            dest[i].setAddr(newp);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

void UserMemory::copyPTs(uptr PDdest,uptr PDsrc, bool start){
    assert(PDdest && PDsrc);
    PageEntry * src = pageHeap.alloc<PageEntry>(PDsrc);
    PageEntry * dest = pageHeap.alloc<PageEntry>(PDdest);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(src[i].present){
            breakpoint;
            dest[i] = src[i];
            uptr newPT = physmemalloc.alloc();
            uptr oldPT = src[i].getAddr();
            dest[i].setAddr(newPT);
            copyPages(newPT,oldPT);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

void UserMemory::copyPDs(uptr PDPdest,uptr PDPsrc){
    assert(PDPdest && PDPsrc);
    PageEntry * src = pageHeap.alloc<PageEntry>(PDPsrc);
    PageEntry * dest = pageHeap.alloc<PageEntry>(PDPdest);
    for(int i = 0 ; i < 512 ; ++i){
        if(src[i].present){
            uptr newPD;
            if(i){
                dest[i] = src[i];
                newPD = physmemalloc.alloc();
                dest[i].setAddr(newPD);
            }
            else{
                newPD = dest[i].getAddr();
            }
            assert(newPD);
            uptr oldPD = src[i].getAddr();
            copyPTs(newPD,oldPD, i == 0);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

UserMemory::UserMemory(const UserMemory& other):UserMemory(){
    copyPDs(_PDP,other._PDP);
}

UserMemory& UserMemory::operator=(const UserMemory& other){
    clear();
    copyPDs(_PDP,other._PDP);
    return *this;
}
void UserMemory::printPages(uptr PT){
    paging.actTmpPT(PT);
    for(int i = 0 ; i < 512 ; ++i){
        if(Paging::tmpPT[i].getAddr()){
            printf ("\t\t\tPage : %d : %p\n",i,Paging::tmpPT[i].getAddr());
        }
    }
    paging.freeTmpPT();
}

void UserMemory::printPTs(uptr PD,bool start){
    paging.actTmpPD(PD);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(Paging::tmpPD[i].getAddr()){
            printf("\t\tPT : %d : %p\n",i,Paging::tmpPD[i].getAddr());
            printPages(Paging::tmpPD[i].getAddr());
        }
    }
    paging.freeTmpPD();
}
void UserMemory::printPDs(uptr PDP){
    paging.actTmpPDP(PDP);
    for(int i = 0 ; i < 512 ; ++i){
        if(Paging::tmpPDP[i].getAddr()){
            printf("\tPD : %d : %p\n",i,Paging::tmpPDP[i].getAddr());
            if(!i)printf("\t\tstandard physical mapping PT\n");
            printPTs(Paging::tmpPDP[i].getAddr(),!i);
        }
    }
    paging.freeTmpPDP();
}

void UserMemory::DumpTree(){
    printf("PDP : %llx\n",_PDP);
    printPDs(_PDP);
}

