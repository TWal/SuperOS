#ifndef KARGS_H
#define KARGS_H

#include "../src/utility.h"

struct OcupArea{
    u64 addr;
    u64 size;
}__attribute__((packed));

struct KArgs{ // all addresses are physical
    u64 occupArea;
    u64 occupAreaSize;
    u64 stackAddr;
    u64 PML4;
    u64 PDPE;
    u64 PDE;
};



#endif
