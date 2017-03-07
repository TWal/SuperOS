#include "../src/Memory/Paging.h"

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
        PML4lower[i].present = false;
        PML4lower[i].readWrite = true;
        PML4lower[i].user = false;
        PML4lower[i].writeThrough = true;
        PML4lower[i].cacheDisable = false;
        PML4lower[i].accessed = false;
        PML4lower[i].zero = false;
        PML4lower[i].isSizeMega = false;
        PML4lower[i].nothing = 0;
        PML4lower[i].addr = 0;
        PML4lower[i].zero2 = 0;
    }
    for(int i = 0 ; i < 512 ; ++i){
        PDPElower[i].present = false;
        PDPElower[i].readWrite = true;
        PDPElower[i].user = false;
        PDPElower[i].writeThrough = true;
        PDPElower[i].cacheDisable = false;
        PDPElower[i].accessed = false;
        PDPElower[i].zero = false;
        PDPElower[i].isSizeMega = false;
        PDPElower[i].nothing = 0;
        PDPElower[i].addr = 0;
        PDPElower[i].zero2 = 0;
    }
    for(int i = 0; i < 512; ++i) {
        PDElower[i].present = false;
        PDElower[i].readWrite = true;
        PDElower[i].user = false;
        PDElower[i].writeThrough = true;
        PDElower[i].cacheDisable = false;
        PDElower[i].accessed = false;
        PDElower[i].zero = false;
        PDElower[i].isSizeMega = false;
        PDElower[i].nothing = 0;
        PDElower[i].addr = 0;
        PDElower[i].zero2 = 0;
    }
    //Identity map the first 2MB
    PML4lower[0].present = true;
    PML4lower[0].setAddr32(PDPElower);
    PDPElower[0].present = true;
    PDPElower[0].setAddr32(PDElower);
    PDElower[0].present = true;
    PDElower[0].isSizeMega = true;
    PDElower[0].addr = 0;

    //For the higher half kernel
    /*   uint PML4offset = HHOFFSET >> (12+9+9+9);
    PML4lower[PML4offset].present = true;
    PML4lower[PML4offset].setAddr32(PDPElower);

    uint PDPEoffset = (HHOFFSET >> (12+9+9)) & ((1 <<9)-1);
    PDPElower[PDPEoffset].present = true;
    PDPElower[PDPEoffset].setAddr32(PDElower);

    for(u32 i = 0; i < MAX_INIT_PAGE; ++i) {

        PDElower[i].present = true;
        PDElower[i].isSizeMega = true;
        PDElower[i].addr = i<<9;
        }*/
}

PageEntry PML4lower[512];
PageEntry PDPElower[512];
PageEntry PDElower [512];
