#ifndef HARDDRIVE_H
#define HARDDRIVE_H

#include "utility.h"
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

class HDD {
    uint _basePort;
    bool _master;
    bool active;
    PartitionTableEntry table [4];


public :
    bool isThereADisk();
    HDD(uchar bus,bool master);
    StatusByte getStatus();
    void setMaster(bool master);
    void setBus(int bus);
    void init();
    void writelba (lint LBA , void* data, uint size);
    void readlba (lint LBA, void * data, uint size);
    void writeaddr(lint addr,void* data,uint size);
    void activate(){}

};



#endif
