#ifndef PARTITION_H
#define PARTITION_H

#include "utility.h"
#include "Bytes.h"

class HDD;

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

class Partition{
    PartitionTableEntry _descriptor;
    HDD*_HDD;
public:
    explicit Partition (HDD*HDD,PartitionTableEntry descriptor);
    ulint getSize(); // for now I don't know how to get this information
    bool isInRAM() {return false ;}
    void* getData(){return nullptr;}
    void writelba (ulint LBA , const void* data, uint nbsector);
    void readlba (ulint LBA, void * data, uint nbsector);
    void writeaddr (ulint addr , const void* data, uint size);
    void readaddr (ulint addr, void * data, uint size);
};


#endif
