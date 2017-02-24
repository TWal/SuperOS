#include "Paging.h"
#include "utility.h"
#include "globals.h"

const uint HF_BEGIN = THREEGB;


PageDirectoryEntry PDElower[1024];
PageDirectoryEntry* PDE = (PageDirectoryEntry*)((uint)PDElower + HF_BEGIN);
PageTable PT[1<<20];
const int MAX_INIT_PAGE = 2;

void PageDirectoryEntry::setAddr(void* a) {
    setAddr((uint)a);
}

void PageDirectoryEntry::setAddr(uint a) {
    assert((a & ((1<<12)-1)) == 0);
    PTaddr = a >> 12;
}

void PageTable::setAddr(void* a) {
    setAddr((uint)a);
}

void PageTable::setAddr(uint a) {
    assert((a & ((1<<12)-1)) == 0);
    addr = a >> 12;
}

void* PageTable::getAddr() {
    return (void*)(addr << 12);
}

void setupBasicPaging() {
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
    for(int i = 0; i < MAX_INIT_PAGE; ++i) {
        PDElower[768+i].present = true;
        PDElower[768+i].isSizeMega = true;
        PDElower[768+i].PTaddr = i<<10;
    }
}

Paging::Paging() : _brk(HF_BEGIN + MAX_INIT_PAGE*4*1024*1024), _truebrk(_brk) {
    for(int i = MAX_INIT_PAGE; i < 256; ++i) {
        PDE[768+i].PTaddr = (((uint)&PT[1024*i])-HF_BEGIN) >> 12;
    }
}

static inline PageTable* getPT(uint addr) {
    return &PT[(addr-HF_BEGIN)>>12];
}

static inline PageDirectoryEntry* getPDE(uint addr) {
    return &PDE[addr >> 22];
}

static inline void invlpg(uint addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

int Paging::brk(void* paddr) {
    uint addr = (uint)paddr;
    if(addr > _truebrk) {
        while(addr > _truebrk) {
            if((_truebrk & ((1<<22)-1)) == 0) {
                for(int i = 0; i < 1024; ++i) {
                    getPT(_truebrk)[i].present = false;
                }
                getPDE(_truebrk)->present = true;
            }
            getPT(_truebrk)->setAddr((uint)physmemalloc.alloc());
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

void* Paging::sbrk(int inc) {
    void* res = (void*)_brk;
    brk(((char*)_brk) + inc);
    return res;
}

static MallocHeader* firstHeader;

static const int FREE = 1;
static const int PREV_FREE = 2;

void initkmalloc() {
    MallocHeader* head = (MallocHeader*)paging.sbrk(4);
    head->size = 0;
    head->flags = 0;
    firstHeader = head;
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

static inline uint align4(uint n) {
    return n + 4 - (n%4);
}

void* kmalloc(uint size) {
    size = align4(size);
    MallocHeader* head = firstHeader;
    while(true) {
        //if we are at the end of the linked list
        if(head->size == 0) {
            //allocate enough space
            assert(headerToPtr(head) == paging.sbrk(4+size));
            //setup header
            head->setSize(size);
            head->flags &= ~FREE;
            //setup the next "end of list" header
            MallocHeader* nextHead = nextHeader(head);
            nextHead->size = 0;
            nextHead->flags = 0;
            //return result
            return headerToPtr(head);
            //we didn't do anything with boundary tags because there is nothing to do.
            //(if the previous block is free, head->flags already contains PREV_FREE)
        }
        //if there is a free block with enough space
        if((head->flags & FREE) != 0 && head->getSize() >= size) {
            //change the flag
            head->flags &= ~FREE;
            //if there is enough space to split this block in two
            if(head->getSize() - size >= 2*sizeof(MallocHeader)) {
                MallocHeader* oldNxtHead = nextHeader(head); //only used in the assert
                uint oldSize = head->getSize();
                //setup the current header
                head->setSize(size);
                MallocHeader* nxtHead = nextHeader(head);
                //setup the header of the next block
                nxtHead->setSize(oldSize - size - sizeof(MallocHeader));
                nxtHead->flags = 0; //it's free and its previous block is allocated
                assert(oldNxtHead == nextHeader(nxtHead));
                //nxtHead is free: copy its boundary tag
                *getNextBoundaryTag(nxtHead) = *nxtHead;
                //oldNxtHead->flags already contains PREV_FREE
                assert((oldNxtHead->flags & PREV_FREE) != 0);
            }
            //remove the PREV_FREE flag of next block
            nextHeader(head)->flags &= ~PREV_FREE;
            return headerToPtr(head);
        }
        head = nextHeader(head);
    }
}

void kfree(void* ptr) {
    MallocHeader* head = ptrToHeader(ptr);
    //free the block
    assert((head->flags & FREE) == 0);
    head->flags |= FREE;

    //set the PREV_FREE flag on the next block
    nextHeader(head)->flags |= PREV_FREE;

    //try to merge with the next block
    MallocHeader* nxtHead = nextHeader(head);
    if(nxtHead->size != 0 && (nxtHead->flags & FREE) != 0) {
        MallocHeader* oldNxtNxtHead = nextHeader(nxtHead); //only used in the assert
        //merge
        head->setSize(head->getSize() + nxtHead->getSize() + sizeof(MallocHeader));
        assert(oldNxtNxtHead == nextHeader(head));
        //copy boundary tag
        *getNextBoundaryTag(head) = *head;
    }

    //try to merge with the previous block
    if((head->flags & PREV_FREE) != 0) {
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
    }
}


