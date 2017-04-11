#include "UserMemory.h"
#include "PhysicalMemoryAllocator.h"
#include "Paging.h"
#include "PageHeap.h"

UserMemory::UserMemory(): _PDP((u64)physmemalloc.alloc()){
    paging.actTmpPDP((void*)_PDP);
    new(tmpPDP) PageEntry[512];
    void* phyPD = physmemalloc.alloc();
    paging.actTmpPD(phyPD);
    new(tmpPD) PageEntry[512];

    tmpPDP[0].activeAddr(phyPD);
    assert(firstPD[0].getAddr());
    tmpPD[0].activeAddr(firstPD[0].getAddr());
    paging.freeTmpPD();
    paging.freeTmpPDP();
}

void UserMemory::activate(){
    paging.switchUser((void*)_PDP);
}

void UserMemory::freePages(void* PT){
    paging.actTmpPT(PT);
    for(int i =  0 ; i < 512 ; ++i){
        if(tmpPT[i].getAddr()&& tmpPT[i].present){
            physmemalloc.free(tmpPT[i].getAddr());
        }
    }
    paging.freeTmpPT();
}

void UserMemory::freePTs(void* PD, bool start){
    paging.actTmpPD(PD);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(tmpPD[i].getAddr()&& tmpPD[i].present){
            freePages(tmpPD[i].getAddr());
            physmemalloc.free(tmpPD[i].getAddr());
            if(start){
                //in this case we are on firstPD : it will remain after free, we must clean it.
                tmpPD[i].present = false;
            }
        }
    }
    paging.freeTmpPD();
}

void UserMemory::freePDs(void* PDP){
    paging.actTmpPDP(PDP);
    freePTs(tmpPDP[0].getAddr(),true);
    for(int i = 1 ; i < 512 ; ++i){
        void * PD;
        if( (PD = tmpPDP[i].getAddr()) && tmpPDP[i].present ){
            freePTs(PD,false);
            physmemalloc.free(PD);
            tmpPDP[i].present = false;
        }
    }
    paging.freeTmpPDP();
}
void UserMemory::clear(){
    assert((void*)_PDP != pagePT[6].getAddr()); // ensure we do not delete the kernel userPDP.
    if((void*)_PDP == PML4[0].getAddr()){ // if this mapping is active return to the default mapping
        paging.switchUser(pagePT[6].getAddr());
    }
    freePDs((void*)_PDP);
}

UserMemory::~UserMemory(){
    clear();
    // Deleting first PD
    paging.actTmpPDP((void*)_PDP);
    physmemalloc.free(tmpPDP[0].getAddr());
    paging.freeTmpPDP();
    // Deleting PDP.
    physmemalloc.free((void*)_PDP);
}

void UserMemory::copyPages(void* PTdest,void*PTsrc){
    assert(PTdest && PTsrc);
    PageTable * src = pageHeap.alloc<PageTable>((u64)PTsrc);
    PageTable * dest = pageHeap.alloc<PageTable>((u64)PTdest);
    for(int i = 0 ; i < 512 ; ++i){
        if(src[i].present){
            dest[i] = src[i];
            void* newp = physmemalloc.alloc(); // TODO copy on demand
            void* oldp = src[i].getAddr();
            void* psrc = pageHeap.alloc<void>((u64)oldp);
            void* pdest = pageHeap.alloc<void>((u64)newp);
            __builtin_memmove(pdest,psrc,0x1000);
            pageHeap.free(psrc);
            pageHeap.free(pdest);
            dest[i].setAddr(newp);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

void UserMemory::copyPTs(void* PDdest,void*PDsrc, bool start){
    assert(PDdest && PDsrc);
    PageEntry * src = pageHeap.alloc<PageEntry>((u64)PDsrc);
    PageEntry * dest = pageHeap.alloc<PageEntry>((u64)PDdest);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(src[i].present){
            breakpoint;
            dest[i] = src[i];
            void* newPT = physmemalloc.alloc();
            void* oldPT = src[i].getAddr();
            dest[i].setAddr(newPT);
            copyPages(newPT,oldPT);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

void UserMemory::copyPDs(void* PDPdest,void*PDPsrc){
    assert(PDPdest && PDPsrc);
    PageEntry * src = pageHeap.alloc<PageEntry>((u64)PDPsrc);
    PageEntry * dest = pageHeap.alloc<PageEntry>((u64)PDPdest);
    for(int i = 0 ; i < 512 ; ++i){
        if(src[i].present){
            void* newPD;
            if(i){
                dest[i] = src[i];
                newPD = physmemalloc.alloc();
                dest[i].setAddr(newPD);
            }
            else{
                newPD = dest[i].getAddr();
            }
            assert(newPD);
            void* oldPD = src[i].getAddr();
            copyPTs(newPD,oldPD, i == 0);
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

UserMemory::UserMemory(const UserMemory& other):UserMemory(){
    copyPDs((void*)_PDP,(void*)other._PDP);
}

UserMemory& UserMemory::operator=(const UserMemory& other){
    clear();
    copyPDs((void*)_PDP,(void*)other._PDP);
    return *this;
}
void UserMemory::printPages(void* PT){
    paging.actTmpPT(PT);
    for(int i = 0 ; i < 512 ; ++i){
        if(tmpPT[i].getAddr()){
            printf ("\t\t\tPage : %d : %p\n",i,tmpPT[i].getAddr());
        }
    }
    paging.freeTmpPT();
}

void UserMemory::printPTs(void* PD,bool start){
    paging.actTmpPD(PD);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(tmpPD[i].getAddr()){
            printf("\t\tPT : %d : %p\n",i,tmpPD[i].getAddr());
            printPages(tmpPD[i].getAddr());
        }
    }
    paging.freeTmpPD();
}
void UserMemory::printPDs(void* PDP){
    paging.actTmpPDP(PDP);
    for(int i = 0 ; i < 512 ; ++i){
        if(tmpPDP[i].getAddr()){
            printf("\tPD : %d : %p\n",i,tmpPDP[i].getAddr());
            if(!i)printf("\t\tstandard physical mapping PT\n");
            printPTs(tmpPDP[i].getAddr(),!i);
        }
    }
    paging.freeTmpPDP();
}

void UserMemory::DumpTree(){
    printf("PDP : %p\n",_PDP);
    printPDs((void*)_PDP);
}
