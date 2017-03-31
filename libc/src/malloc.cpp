#include"../stdlib.h"
#include"malloc.h"
#include "../stdio.h"

static MallocHeader* firstHeader;

void initmalloc() {
    firstHeader = (MallocHeader*)sbrk(sizeof(MallocHeader));
    firstHeader->size = 0;
    firstHeader->prevFree = false;
    firstHeader->free = false;
    firstHeader->update();
}

static inline void* headerToPtr(MallocHeader* head) {
    return (void*)((char*)head + sizeof(MallocHeader));
}

static inline MallocHeader* ptrToHeader(void* ptr) {
    auto res = (MallocHeader*)((char*)ptr - sizeof(MallocHeader));
    assert(res->seemsValid());
    return res;
}

static inline MallocHeader* getPrevBoundaryTag(MallocHeader* head) {
    assert(head->prevFree && head->seemsValid());
    auto res = (MallocHeader*)((char*)head - sizeof(MallocHeader));
    assert(res->seemsValid());
    return res;
}

static inline MallocHeader* getNextBoundaryTag(MallocHeader* head) {
    assert(head->free && head->seemsValid());
    return (MallocHeader*)((char*)head + head->getSize());

}

static inline MallocHeader* nextHeader(MallocHeader* head,bool init = false);
static inline MallocHeader* nextHeader(MallocHeader* head,bool init) {
    assert(head->seemsValid());
    auto res = (MallocHeader*)((char*)head + head->getSize() + sizeof(MallocHeader));
    assert(init || res->seemsValid());
    return res;
}


static inline size_t align8(size_t n) {
    return  (n + 7) / 8 * 8;
}

void* malloc(size_t size) {
    //printf("malloc %lld",size);
    size = align8(size);
    MallocHeader* head = firstHeader;
    assert(head->seemsValid());
    while(true) {
        //if we are at the end of the linked list
        if(head->size == 0) {
            //allocate enough space
            assert(headerToPtr(head) == sbrk(sizeof(MallocHeader)+size));
            //setup header
            head->setSize(size);
            head->setFree(false);
            //setup the next "end of list" header
            MallocHeader* nextHead = nextHeader(head,true);
            nextHead->size = 0;
            nextHead->prevFree = false;
            nextHead->free = false;
            nextHead->update();
            //return result
            return headerToPtr(head);
            //we didn't do anything with boundary tags because there is nothing to do.
            //(if the previous block is free, head->prevFree is already true)
        }
        //if there is a free block with enough space
        if(head->free && head->getSize() >= size) {
            //change the flag
            head->setFree(false);
            //if there is enough space to split this block in two
            if(head->getSize() - size >= 2*sizeof(MallocHeader)) {
                MallocHeader* oldNxtHead = nextHeader(head); //only used in the assert
                size_t oldSize = head->getSize();
                //setup the current header
                head->setSize(size);
                MallocHeader* nxtHead = nextHeader(head,true);
                //setup the header of the next block
                nxtHead->setSize(oldSize - size - sizeof(MallocHeader));
                //it's free and its previous block is allocated
                nxtHead->free = true;
                nxtHead->prevFree = false;
                nxtHead->update();
                // check arithmetic
                assert(oldNxtHead == nextHeader(nxtHead));
                //nxtHead is free: copy its boundary tag
                *getNextBoundaryTag(nxtHead) = *nxtHead;
                assert(getNextBoundaryTag(nxtHead) == getPrevBoundaryTag(nextHeader(nxtHead)));
                //oldNxtHead->prevFree is true
                assert(oldNxtHead->prevFree);
            }
            //remove the PREV_FREE flag of next block
            nextHeader(head)->setPFree(false);
            return headerToPtr(head);
        }
        head = nextHeader(head);
    }
}

void free(void* ptr) {
    printf("freeing %p",ptr);
    MallocHeader* head = ptrToHeader(ptr);
    //free the block
    assert(!head->free && "This is probably a double free!");
    head->setFree(true);

    //set the PREV_FREE flag on the next block
    nextHeader(head)->setPFree(true);

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
        head = prevHead; // prevHead is now the head of the current block.

    } else { // if fusion to next or no fusion
        //copy boundary tag
        *getNextBoundaryTag(head) = *head;
    }

    //freeing heap ...
    nxtHead = nextHeader(head);
    if(nxtHead->size == 0){
        //we are in a free block at the end we are really freeing it :
        head->free =0; // head is now the new end block
        head->prevFree = 0;
        head->update();
        sbrk(-(intptr_t)(head->getSize() + sizeof(MallocHeader)));
        head->size = 0;
        // the freed block is destroy and memory is released to sbrk;
    }



}
