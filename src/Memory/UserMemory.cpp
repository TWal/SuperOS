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
        if(Paging::tmpPD[i].getAddr() and Paging::tmpPD[i].present){
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
        if( (PD = Paging::tmpPDP[i].getAddr()) and Paging::tmpPDP[i].present ){
            freePTs(PD,false);
            physmemalloc.free(PD);
            Paging::tmpPDP[i].present = false;
        }
    }
    paging.freeTmpPDP();
}
void UserMemory::clear(){
    debug(Proc,"Clearing :");
    DumpTree();
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
        else{
            new(dest +i) PageTable();
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
        else{
            new(dest +i) PageEntry();
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
        else{
            new(dest +i) PageEntry();
        }
    }
    pageHeap.free(src);
    pageHeap.free(dest);
}

UserMemory::UserMemory(const UserMemory& other):UserMemory(){
    /*debug(Proc,"before copy :");
      other.DumpTree();*/
    copyPDs(_PDP,other._PDP);
    /*debug(Proc,"after copy original :");
    other.DumpTree();
    debug(Proc,"after copy copied:");
    DumpTree();*/
}

UserMemory& UserMemory::operator=(const UserMemory& other){
    clear();
    /*debug(Proc,"before copy :");
      other.DumpTree();*/
    copyPDs(_PDP,other._PDP);
    /*debug(Proc,"after copy original :");
    other.DumpTree();
    debug(Proc,"after copy copied:");
    DumpTree();*/
    return *this;
}
void UserMemory::printPages(uptr PT){
    PageEntry* tPT = pageHeap.alloc<PageEntry>(PT);
    for(int i = 0 ; i < 512 ; ++i){
        if(tPT[i].getAddr() and tPT[i].present){
            debug(Proc, "\t\t\tPage : %d : %p",i,tPT[i].getAddr());
        }
    }
    pageHeap.free(tPT);
}

void UserMemory::printPTs(uptr PD,bool start){
    PageEntry* tPD = pageHeap.alloc<PageEntry>(PD);
    for(int i = start ? 1 : 0 ; i < 512 ; ++i){
        if(tPD[i].getAddr() and tPD[i].present){
            debug(Proc, "\t\tPT : %d : %p",i,tPD[i].getAddr());
            printPages(tPD[i].getAddr());
        }
    }
    pageHeap.free(tPD);
}
void UserMemory::printPDs(uptr PDP){
    PageEntry* tPDP = pageHeap.alloc<PageEntry>(PDP);
    for(int i = 0 ; i < 512 ; ++i){
        if(tPDP[i].getAddr() and tPDP[i].present){
            debug(Proc, "\tPD : %d : %p", i, tPDP[i].getAddr());
            if(!i)debug(Proc, "\t\tstandard physical mapping PT");
            printPTs(tPDP[i].getAddr(),!i);
        }
    }
    pageHeap.free(tPDP);
}

void UserMemory::DumpTree() const {
    if(serLvls[Proc] < Debug) return;
    debug(Proc, "PDP : %llx", _PDP);
    printPDs(_PDP);
}

