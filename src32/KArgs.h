#ifndef KARGS_H
#define KARGS_H

#include "../src/utility.h"

/**
   @brief This structure is used by KArgs to mark a segment of memory used

   @sa KArgs
 */

struct OccupArea{
    u32 addr; ///< The address of the beginning of the segment.
    u32 nbPages; ///< The number of pages (4K) occupied by the segment.
}__attribute__((packed));



/**
   @class KArgs

   @brief Arguments transmitted from loader to kernel to describe
   the result of the loading process.

   The information given is
       - Occupied Memory (OccupArea table)
       - Stack Position
       - PML4 position (cf. Paging)
       - End of used addresses.
       - Size of RAM

   All addresses are physical addresses.

   @section loadspec Loading Specification

   The kernel is loaded in virtual memory as specified by the elf64 format.

   The OccupArea table describe the physical ram area that are used by kernel code, stack or
   Paging structure.

   The stack Addr is the page of a beginning of the kernel stack, only one page is allocated :
   The kernel must not use more than 4K of stack before setting up its own.

   The PML4 field is the physical address of PML4.


   It is guaranteed that there is an identity mapping of enough 2MB pages to ensure
   that all those physical addresses are identity mapped at kernel launching. (6MB currently)

   The kernel must setup its own GDT before allocating memory on unoccupied pages
   because the page where the GDT is is maked as free.

   The code of the loader is also marked as free.

   @sa OccupArea

*/

struct KArgs{
    u64 occupArea; ///< Begining of the OccupArea table.
    u64 occupAreaSize; ///< Number of Segment in OccupArea table.
    u64 stackAddr; ///< Stack position
    u64 PML4; ///< PML4 position
    u64 freeAddr; ///< First free adress : no memory is used by the loader beyond this point.
    u64 RAMSize; ///< The RAMSize except the first 1MB
}__attribute__((packed));




#endif
