#ifndef PARTITION_H
#define PARTITION_H

#include "utility.h"

struct CHS {
    uchar Head;
    uchar sector : 6;
    uint cylinder : 10;
}__attribute((packed));

static_assert (sizeof(CHS) == 3, "");

struct PartitionTableEntry{
    uint zero1 : 4;
    bool bootable : 1;
    uint zero2 : 3;
    CHS begchs;
    uchar systemID;
    CHS endchs;
    uint begLBA;
    uint endLBA;
} __attribute((packed));


static_assert (sizeof(PartitionTableEntry) == 16, "");


#endif
