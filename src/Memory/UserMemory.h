#ifndef USERMEMORY_H
#define USERMEMORY_H

#include"../utility.h"
#include"../Bitset.h"
#include "Paging.h"
#include "../log.h"

//#define NBUSERSTACK 1

/**
   @brief Represent user space memory i.e a user PDP.

   The class mainly contains a valid user PDP (see @ref map_user).
   In fact the class owns as fixed allocated variables _PDP and the first
   PD : @code _PDP[0].getAddr() @endcode

   Thus the class invariant is that we have a valid user PDP ie. the first PD is always active,
   It is setup in slot 0 of the PDP, and have its own slot 0 setup to the identity map PT.

   All other structure are dynamic and not tied to the object are copied
   (@ref operator=) or cleared (@ref clear).

 */

class UserMemory{
    uptr _PDP; ///< The user PDP owned by this object.
    //u64 _bitdata[NBUSERSTACK]; idea for multiple non-heap thread stack
    //Bitset _bitset;
    /**
       @brief Free all pages from one PT
       @param PT The physical address of the PT.

       Just take all active physical pages of the PT and frees them with
       PhysicalMemoryAllocator::free.
     */
    static void freePages(uptr PT);
    /**
       @brief Free all pages and PTs from one PD
       @param PD The physical address of the PD.
       @param start If true, Ignore the first PT

       Just take all active PT of the PD and call freePages on them, then frees them with
       PhysicalMemoryAllocator::free.

       The flag start is meant for the first PD (we should not destroy the identity map PT)
    */
    static void freePTs(uptr PD, bool start);
    /**
       @brief Free all pages, PTs and PDs from one PDP
       @param PDP The physical address of the PDP.

       Just take all active PD of the PD and call freePDs on them, then frees them with
       PhysicalMemoryAllocator::free.

       It just needs to send start = true for the first one, and to not delete it.
    */
    static void freePDs(uptr PDP);
    static void copyPages(uptr PTdest,uptr PTsrc); // copy untested
    static void copyPTs(uptr PDdest,uptr PDsrc, bool start);
    static void copyPDs(uptr PDPdest,uptr PDPsrc);
    static void printPages(uptr PT);
    static void printPTs(uptr PD,bool start);
    static void printPDs(uptr PDP);

public :
    /**
       @brief Create an empty user PDP similar to the default one
     */
    UserMemory();
    /**
       @brief Create a new user PDP using an other one, see @ref operator=
    */
    UserMemory(const UserMemory& other);

    /**
       @brief Just @ref clear the PDP then delete the PDP and its first PD.
    */
    ~UserMemory();
    void activate(); ///< Uses Paging::switchUser to activate itself.
    /**
       @brief Clear all mappings of the object

       The object is now in the same state than when it is freshly constructed.

       In fact just call freePDs(_PDP).
     */
    void clear();
    /**
       @brief Perform a deep copy of the other UserMemory.

       Currently do not implement copy on write, This function just copy all
       paging structure and physical pages in freshly allocated new pages.
     */
    UserMemory& operator=(const UserMemory& other);
    void DumpTree() const ; ///< Print the mappings of this objects.
    bool in(const void* addr) const; // check if this address is a valid address

};


#endif
