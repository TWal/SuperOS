#ifndef HARDDRIVE_H
#define HARDDRIVE_H

#include "utility.h"
#include "Bytes.h"
#include "Partition.h"



struct StatusByte{
    StatusByte(u8 byte);
    bool error :1;
    int zero : 2;
    bool drq : 1;
    bool srv : 1;
    bool driveFault : 1;
    bool ready : 1;
    bool busy : 1;
    bool isReadyOrFailed(){return error || driveFault || !busy;}
    bool isOk(){return !(error || driveFault);}
    void printStatus();

}__attribute((packed));

class HDD : public HDDBytes {
    u16 _basePort;
    bool _master;
    bool active;
    PartitionTableEntry table [4];
    u8 MBR[512];

public :
    size_t getSize() {return 0;}; // for now I don't know how to get this information
    bool isInRAM() {return false ;}
    void* getData(){return nullptr;}

    bool isThereADisk();
    HDD(u8 bus,bool master);
    StatusByte getStatus();
    void setMaster(bool master);
    void setBus(int bus);
    void init();
    void writelba (u32 LBA , const void* data, u32 nbsector);
    void readlba (u32 LBA, void * data, u32 nbsector);
    void writeaddr (uptr addr , const void* data, size_t size);
    void readaddr (uptr addr, void * data, size_t size);
    void activate(){}
    PartitionTableEntry operator[](u8 i);

};



#endif
