#ifndef USERMEMORY_H
#define USERMEMORY_H

#include"../utility.h"
#include"../Bitset.h"

//#define NBUSERSTACK 1

class UserMemory{
    u64 _PDP;
    //u64 _bitdata[NBUSERSTACK]; idea for multiple non-heap thread stack
    //Bitset _bitset;
    static void freePages(void* PT);
    static void freePTs(void* PD, bool start);
    static void freePDs(void* PDP);
    static void copyPages(void* PTdest,void*PTsrc); // copy untested
    static void copyPTs(void* PDdest,void*PDsrc, bool start);
    static void copyPDs(void* PDPdest,void*PDPsrc);
    static void printPages(void* PT);
    static void printPTs(void* PD,bool start);
    static void printPDs(void* PDP);

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
