#include "Paging.h"
#include "utility.h"

PageDirectoryEntry PDElower[1024];
PageDirectoryEntry* PDE = PDElower + 0xC0000000;

void setupPaging() {
    for(int i = 0; i < 1024; ++i) {
        PDElower[i].present = false;
        PDElower[i].readWrite = true;
        PDElower[i].user = false;
        PDElower[i].writeThrough = true;
        PDElower[i].cacheDisable = false;
        PDElower[i].accessed = false;
        PDElower[i].zero = false;
        PDElower[i].isSizeMega = false;
        PDElower[i].nothing = 0;
        PDElower[i].PTaddr = 0;
    }
    //Identity map the first 4Mb
    PDElower[0].present = true;
    PDElower[0].isSizeMega = true;
    PDElower[0].PTaddr = 0;

    //For the higher half kernel
    PDElower[768].present = true;
    PDElower[768].isSizeMega = true;
    PDElower[768].PTaddr = 0;
}
