#include <string.h>
#include "HardDrive.h"
#include "globals.h"

static const uint bus1BasePort = 0x1F0;
static const uint bus1SpecialPort = 0x3F6;
static const uint bus2BasePort = 0x170;
static const uint bus2SpecialPort = 0x376;


StatusByte::StatusByte(uchar byte){
    *reinterpret_cast<uchar*>(this) = byte;
}

void StatusByte::printStatus(){
    fb.printf("byte: %u,error : %d, ready : %d, busy : %d ",
              *reinterpret_cast<uchar*>(this),error,ready,busy);
}

StatusByte HDD::getStatus(){
    return inb(_basePort + 7);
}

bool HDD::isThereADisk(){
    return inb(_basePort + 7) != 0xFF;
}

void HDD::init(){
    readlba(0,MBR,1);
    memcpy(table,MBR+0x1be,4*sizeof(PartitionTableEntry));
}

HDD::HDD(uchar bus,bool master) : _master(master),active(false),table{},MBR{}
{
    switch(bus){
        case 1:
            _basePort = bus1BasePort;
            break;
        case 2:
            _basePort = bus2BasePort;
            break;
        default:
            bsod("Unknown IDE bus number : %d\n",bus);
    }
}


void HDD::writelba (ulint LBA , const void* data, uint nbsector){
    assert(nbsector > 0 && nbsector < 512 && data != nullptr);
    activate();
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, (LBA >> 32) & 0xFF);
    outb (_basePort + 5, (LBA >> 40) & 0xFF);
    outb (_basePort + 2, nbsector & 0xFF);
    outb (_basePort + 3, LBA & 0xFF);
    outb (_basePort + 4, (LBA >> 8) & 0xFF);
    outb (_basePort + 5, (LBA >> 16) & 0xFF);
    outb (_basePort + 7,0x34);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
    for(uint i = 0 ; i < nbsector*256 ; ++ i ){
        outw(_basePort,reinterpret_cast<const ushort*>(data)[i]);
    }



    outb (_basePort + 7,0xE7);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
}


void HDD::readlba (ulint LBA, void * data, uint nbsector){
    readaddr(LBA*512,data,nbsector*512);

}

void HDD::writeaddr (ulint addr , const void* data, uint size){
    static uchar buffer[512];
    lint LBA = addr/512;
    uint offset = addr %512;
    if (offset != 0){
        readlba(LBA,buffer,1);
        uint toCopy = min(512-offset,size);
        memcpy(buffer+offset,data,toCopy);
        writelba(LBA,buffer,1);
        if(toCopy == size) return;
        ++LBA;
        data = reinterpret_cast<const uchar*>(data) + toCopy;
        size -= (512-offset);
    }
    uint toWriteSectors = size /512;
    if(toWriteSectors > 0){
        writelba(LBA,data,toWriteSectors);
        size -= toWriteSectors * 512;
        data = reinterpret_cast<const uchar*>(data) + 512 * toWriteSectors;
    }
    if (size > 0){
        readlba(LBA,buffer,1);
        memcpy(buffer,data,size);
        writelba(LBA,buffer,1);
    }

}
void HDD::readaddr (ulint addr, void * data, uint size){
    assert(size > 0 && size < (512*512) && ((size %2) == 0));
    activate();
    ushort nbsector = (size + 511) / 512;
    lint LBA = addr/512;
    uint offset = addr %512;
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, (LBA >> 32) & 0xFF);
    outb (_basePort + 5, (LBA >> 40) & 0xFF);
    outb (_basePort + 2, nbsector & 0xFF);
    outb (_basePort + 3, LBA & 0xFF);
    outb (_basePort + 4, (LBA >> 8) & 0xFF);
    outb (_basePort + 5, (LBA >> 16) & 0xFF);
    outb (_basePort + 7,0x24);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
    while (offset >=2){
        inw(_basePort);
        offset -=2;
    }
    if(offset == 1){
        ushort d = inw(_basePort);
        *reinterpret_cast<uchar*>(data) = d >> 8;
        data = reinterpret_cast<uchar*>(data) +1;
        size--;
    }
    for(volatile int i = 0 ; i < 100000000 ; ++i);
    for( ;size > 0; size -=2 ){

        *reinterpret_cast<ushort*>(data) = inw(_basePort);
        data = reinterpret_cast<ushort*>(data) +1;
    }
    size +=2;
    if (size == 1){
        ushort d = inw(_basePort);
        *reinterpret_cast<uchar*>(data) = uchar(d);
    }

}

PartitionTableEntry HDD::operator[](int i){
    assert(i>0 && i <=4);
    return table[i-1];
}
