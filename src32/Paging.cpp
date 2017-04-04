#include "../src/Memory/Paging.h"
#include "../src/IO/FrameBuffer.h"

const uint MAX_INIT_PAGE = 4;

void PageEntry::setAddr32(u32 a){
    assert((a & ((1<<12)-1)) == 0);
    addr = a >> 12;
}

void PageEntry::setAddr32(void* a){
    setAddr32((u32)a);
}

void setupBasicPaging() {
    for(int i = 0 ; i < 512 ; ++i){
        PML4[i].present = false;
        PML4[i].readWrite = true;
        PML4[i].user = true;
        PML4[i].writeThrough = false;
        PML4[i].cacheDisable = false;
        PML4[i].accessed = false;
        PML4[i].zero = false;
        PML4[i].isSizeMega = false;
        PML4[i].nothing = 0;
        PML4[i].addr = 0;
        PML4[i].zero2 = 0;
    }
    for(int i = 0; i < PDP_NUM; ++i) {
        for(int j = 0 ; j < 512 ; ++j){
            PDPs[i][j].present = false;
            PDPs[i][j].readWrite = true;
            PDPs[i][j].user = true;
            PDPs[i][j].writeThrough = false;
            PDPs[i][j].cacheDisable = false;
            PDPs[i][j].accessed = false;
            PDPs[i][j].zero = false;
            PDPs[i][j].isSizeMega = false;
            PDPs[i][j].nothing = 0;
            PDPs[i][j].addr = 0;
            PDPs[i][j].zero2 = 0;
        }
    }
    for(int i = 0; i < PD_NUM; ++i) {
        for(int j = 0; j < 512; ++j) {
            PDs[i][j].present = false;
            PDs[i][j].readWrite = true;
            PDs[i][j].user = true;
            PDs[i][j].writeThrough = false;
            PDs[i][j].cacheDisable = false;
            PDs[i][j].accessed = false;
            PDs[i][j].zero = false;
            PDs[i][j].isSizeMega = false;
            PDs[i][j].nothing = 0;
            PDs[i][j].addr = 0;
            PDs[i][j].zero2 = 0;
        }
    }
    for(int i = 0; i < PT_NUM; ++i) {
        for(int j = 0 ; j < 512 ; ++j) {
            PTs[i][j].present = false;
            PTs[i][j].readWrite = true;
            PTs[i][j].user = true;
            PTs[i][j].writeThrough = false;
            PTs[i][j].cacheDisable = false;
            PTs[i][j].accessed = false;
            PTs[i][j].dirty = false;
            PTs[i][j].zero = false;
            PTs[i][j].global = false;
            PTs[i][j].nothing = 0;
            PTs[i][j].addr = 0;
            PTs[i][j].zero2 = 0;
        }
    }
    //Identity map the first 2MB (loader must not use more than 1 MB)
    PML4[0].present = true;
    PML4[0].setAddr32(PDPs[0]);
    ++nbPDPused;
    PDPs[0][0].present = true;
    PDPs[0][0].setAddr32(PDs[0]);
    ++nbPDused;
    PDs[0][0].present = true;
    PDs[0][0].isSizeMega = true;
    PDs[0][0].writeThrough = true;
    PDs[0][0].addr = 0;
    ++nbPDused;
    PDs[0][1].present = true;
    PDs[0][1].isSizeMega = true;
    PDs[0][1].setAddr(0x200000);

}

PageEntry* getPDPphyu(u64 addr){ // unsafe version
    return reinterpret_cast<PageEntry*> (PML4[getPML4index(addr)].getAddr());
}

PageEntry* getPDPphy(u64 addr){ // safe version
    auto PDP = reinterpret_cast<PageEntry*> (PML4[getPML4index(addr)].getAddr());
    //printf("Accessing PML4 at %d %p\n",getPML4index(addr),PDP);
    if(PDP) return PDP;
    assert(nbPDPused < PDP_NUM);
    PML4[getPML4index(addr)].setAddr(PDPs[nbPDPused]);
    PML4[getPML4index(addr)].present = true;
    ++nbPDPused;
    return PDPs[nbPDPused-1];
}

inline PageEntry* getPDphyu(u64 addr){ // unsafe version
    return reinterpret_cast<PageEntry*> (getPDPphyu(addr)[getPDPindex(addr)].getAddr());
}
PageEntry* getPDphy(u64 addr){ // safe version
    auto PDP = getPDPphy(addr);
    auto PD = reinterpret_cast<PageEntry*> (PDP[getPDPindex(addr)].getAddr());
    //printf("Accessing PDP at %d %p\n",getPDPindex(addr),PD);
    if(PD) return PD;
    assert(nbPDused < PD_NUM);
    //printf("PDs %p, this : %p",PDs,PDs[nbPDused]);
    PDP[getPDPindex(addr)].setAddr(PDs[nbPDused]);
    PDP[getPDPindex(addr)].present = true;
    ++nbPDused;
    return PDs[nbPDused-1];
}

inline PageTable* getPTphyu(u64 addr){ // unsafe version
    return reinterpret_cast<PageTable*> (getPDphyu(addr)[getPDindex(addr)].getAddr());
}
PageTable* getPTphy(u64 addr){ // safe version 
    auto PD = getPDphy(addr);
    auto PT = reinterpret_cast<PageTable*> (PD[getPDindex(addr)].getAddr());
    //printf("Accessing PD at %d %p\n",getPDindex(addr),PT);
    if(PT) return PT;
    assert(nbPTused < PT_NUM);
    PD[getPDindex(addr)].setAddr(PTs[nbPTused]);
    PD[getPDindex(addr)].present = true;
    ++nbPTused;
    return PTs[nbPTused-1];
}

inline void* getphyu(u64 addr){ // unsafe version
    return getPTphyu(addr)[getPTindex(addr)].getAddr();
}
// there is no safe version getphy.



void createMapping(void* phy,u64 virt){
    assert(!(u64(phy) & ((1<< 12 )-1)) && !(virt & ((1<< 12 )-1)));
    auto PT = getPTphy(virt);
    PT[getPTindex(virt)].setAddr(phy);
    PT[getPTindex(virt)].present =true;
}
void createMapping(void* phy,u64 virt,int numPg){
    char* cphy = (char*)phy;
    for(int i = 0 ; i < numPg ; ++i){
        //breakpoint;
        createMapping(cphy + i * 0x1000,virt+i*0X1000);
    }
}



PageEntry PML4[512];
PageEntry PDPs[PDP_NUM][512];
PageEntry PDs[PD_NUM][512];
PageTable PTs[PT_NUM][512];
u32 nbPDPused = 0 ;
u32 nbPDused = 0;
u32 nbPTused = 0;

