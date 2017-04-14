#ifndef USERMEMORY_H
#define USERMEMORY_H

#include"../utility.h"
#include"../Bitset.h"
#include "Paging.h"

//#define NBUSERSTACK 1

class UserMemory{
    uptr _PDP;
    //u64 _bitdata[NBUSERSTACK]; idea for multiple non-heap thread stack
    //Bitset _bitset;
    static void freePages(uptr PT);
    static void freePTs(uptr PD, bool start);
    static void freePDs(uptr PDP);
    static void copyPages(uptr PTdest,uptr PTsrc); // copy untested
    static void copyPTs(uptr PDdest,uptr PDsrc, bool start);
    static void copyPDs(uptr PDPdest,uptr PDPsrc);
    static void printPages(uptr PT);
    static void printPTs(uptr PD,bool start);
    static void printPDs(uptr PDP);

public :
    UserMemory();
    UserMemory(const UserMemory& other);
    ~UserMemory();
    void activate();
    void clear(); // the object is new again
    UserMemory& operator=(const UserMemory& other); // copy untested
    void DumpTree(); // DumpMemory tree.

};


#endif
