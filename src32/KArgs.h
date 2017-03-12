#ifndef KARGS_H
#define KARGS_H

#include "../src/utility.h"

struct OccupArea{
    u32 addr;
    u32 nbPages;
}__attribute__((packed));

struct KArgs{ // all addresses are physical
    u64 occupArea;
    u64 occupAreaSize;
    u64 stackAddr;
    u64 PML4;
}__attribute__((packed));

/* ----------------LOADING SPECIFICATION----------------------------

   The kernel is loaded in virtual memory as specified by the elf64 format.
   The OccupArea table describe the physical ram area that are used by kernel code, stack or
   Paging or segmenting structure

   The stack Addr is the page of a beginning of the kernel stack, only one page is allocated
   The PML4 field is the physical address of PML4


   it is guaranteed that there is an identity mapping of enough 2MB pages to ensure
   that all those physical addresses are identity mapped at kernel launching.

   The kernel must setup its own GDT before allocating memory on unoccupied pages.

*/



#endif
