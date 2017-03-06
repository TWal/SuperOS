#include "Paging.h"
#include "PhysicalMemoryAllocator.h"

PageDirectoryEntry PDElower[1024];
PageDirectoryEntry* PDE = (PageDirectoryEntry*)((uint)PDElower + THREEGB);
PageTable PT[1<<20];
const uint MAX_INIT_PAGE = 2;

void PageDirectoryEntry::setAddr(void* a) {
    setAddr((uptr)a);
}

void PageDirectoryEntry::setAddr(uptr a) {
    assert((a & ((1<<12)-1)) == 0);
    PTaddr = a >> 12;
}

void PageTable::setAddr(void* a) {
    setAddr((uptr)a);
}

void PageTable::setAddr(uptr a) {
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
    for(u32 i = 0; i < MAX_INIT_PAGE; ++i) {
        PDElower[768+i].present = true;
        PDElower[768+i].isSizeMega = true;
        PDElower[768+i].PTaddr = i<<10;
    }
}

Paging::Paging() : _brk(THREEGB + (MAX_INIT_PAGE+4)*4*1024*1024), _truebrk(_brk) {
    for(u32 i = (MAX_INIT_PAGE+4); i < 256; ++i) {
        PDE[768+i].setAddr((((uint)&PT[1024*i])-THREEGB));
    }
}

static inline PageTable* getPT(uptr addr) {
    return &PT[(addr-THREEGB)>>12];
}

static inline PageDirectoryEntry* getPDE(uptr addr) {
    return &PDE[addr >> 22];
}

static inline void invlpg(uptr addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

int Paging::brk(void* paddr) {
    uptr addr = (uptr)paddr;
    if(addr > _truebrk) {
        while(addr > _truebrk) {
            if((_truebrk & ((1<<22)-1)) == 0) {
                for(int i = 0; i < 1024; ++i) {
                    getPT(_truebrk)[i].present = false;
                }
                getPDE(_truebrk)->present = true;
            }
            getPT(_truebrk)->setAddr((uptr)physmemalloc.alloc());
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

void* Paging::sbrk(size_t inc) {
    void* res = (void*)_brk;
    brk(((char*)_brk) + inc);
    return res;
}

static MallocHeader* firstHeader;

//static const int FREE = 1;
//static const int PREV_FREE = 2;

void initkmalloc() {
    firstHeader = (MallocHeader*)paging.sbrk(4);
    firstHeader->size = 0;
    firstHeader->prevFree = false;
    firstHeader->free = false;
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

static inline size_t align4(size_t n) {
    return n + 4 - (n%4);
}

void* kmalloc(size_t size) {
    size = align4(size);
    MallocHeader* head = firstHeader;
    while(true) {
        //if we are at the end of the linked list
        if(head->size == 0) {
            //allocate enough space
            assert(headerToPtr(head) == paging.sbrk(4+size));
            //setup header
            head->setSize(size);
            head->free = false;
            //setup the next "end of list" header
            MallocHeader* nextHead = nextHeader(head);
            nextHead->size = 0;
            nextHead->prevFree = false;
            nextHead->free = false;
            //return result
            return headerToPtr(head);
            //we didn't do anything with boundary tags because there is nothing to do.
            //(if the previous block is free, head->prevFree is already true)
        }
        //if there is a free block with enough space
        if(head->free && head->getSize() >= size) {
            //change the flag
            head->free = false;
            //if there is enough space to split this block in two
            if(head->getSize() - size >= 2*sizeof(MallocHeader)) {
                MallocHeader* oldNxtHead = nextHeader(head); //only used in the assert
                size_t oldSize = head->getSize();
                //setup the current header
                head->setSize(size);
                MallocHeader* nxtHead = nextHeader(head);
                //setup the header of the next block
                nxtHead->setSize(oldSize - size - sizeof(MallocHeader));
                //it's free and its previous block is allocated
                nxtHead->free = true;
                nxtHead->prevFree = false;
                assert(oldNxtHead == nextHeader(nxtHead));
                //nxtHead is free: copy its boundary tag
                *getNextBoundaryTag(nxtHead) = *nxtHead;
                assert(getNextBoundaryTag(nxtHead) == getPrevBoundaryTag(nextHeader(nxtHead)));
                //oldNxtHead->prevFree is true
                assert(oldNxtHead->prevFree);
            }
            //remove the PREV_FREE flag of next block
            nextHeader(head)->prevFree = false;
            return headerToPtr(head);
        }
        head = nextHeader(head);
    }
}

void kfree(void* ptr) {
    if(ptr == nullptr) return;

    MallocHeader* head = ptrToHeader(ptr);
    //free the block
    assert(!head->free && "This is probably a double free!");
    head->free = true;

    //set the PREV_FREE flag on the next block
    nextHeader(head)->prevFree = true;

    //try to merge with the next block
    MallocHeader* nxtHead = nextHeader(head);
    if(nxtHead->size != 0 && nxtHead->free) {
        MallocHeader* oldNxtNxtHead = nextHeader(nxtHead); //only used in the assert
        //merge
        head->setSize(head->getSize() + nxtHead->getSize() + sizeof(MallocHeader));
        assert(oldNxtNxtHead == nextHeader(head));
    }

    //try to merge with the previous block
    if(head->prevFree) {
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
    } else {
        //copy boundary tag
        *getNextBoundaryTag(head) = *head;
    }
}
Paging paging;


