#ifndef PARTITION_H
#define PARTITION_H

#include "../utility.h"
#include "../Bytes.h"

class HDD;

struct CHS {
    u8 Head;
    u8 sector : 6;
    u16 cylinder : 10;
}__attribute((packed));

static_assert (sizeof(CHS) == 3, "");

struct PartitionTableEntry{
    u8 zero1 : 4;
    bool bootable : 1;
    u8 zero2 : 3;
    CHS begchs;
    u8 systemID;
    CHS endchs;
    u32 begLBA;
    u32 size;
} __attribute((packed));


static_assert (sizeof(PartitionTableEntry) == 16, "");

class Partition : public HDDBytes {
    PartitionTableEntry _descriptor;
    HDD*_HDD;
public:
    explicit Partition (HDD*HDD,PartitionTableEntry descriptor);
    size_t getSize()const;
    bool isInRAM()const {return false ;}
    void* getData(){return nullptr;}
    void writelba (u32 LBA , const void* data, u32 nbsector);
    void readlba (u32 LBA, void * data, u32 nbsector) const;
    void writeaddr (u64 addr , const void* data, size_t size);
    void readaddr (u64 addr, void * data, size_t size) const;
};


#endif
