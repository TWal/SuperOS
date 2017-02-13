#ifndef HARDDRIVE_H
#define HARDDRIVE_H

#include "utility.h"
#include "Bytes.h"
#include "Partition.h"



struct StatusByte{
    StatusByte(uchar byte);
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
    uint _basePort;
    bool _master;
    bool active;
    PartitionTableEntry table [4];
    uchar MBR[512];

public :
    ulint getSize() {return 0;}; // for now I don't know how to get this information
    bool isInRAM() {return false ;}
    void* getData(){return nullptr;}

    bool isThereADisk();
    HDD(uchar bus,bool master);
    StatusByte getStatus();
    void setMaster(bool master);
    void setBus(int bus);
    void init();
    void writelba (ulint LBA , const void* data, uint nbsector);
    void readlba (ulint LBA, void * data, uint nbsector);
    void writeaddr (ulint addr , const void* data, uint size);
    void readaddr (ulint addr, void * data, uint size);
    void activate(){}
    PartitionTableEntry operator[](int i);

};



#endif
