#include <string.h>
#include "HardDrive.h"
#include <stdio.h>

static const u16 bus1BasePort = 0x1F0;
static const u16 bus1SpecialPort = 0x3F6;
static const u16 bus2BasePort = 0x170;
static const u16 bus2SpecialPort = 0x376;


StatusByte::StatusByte(u8 byte){
    *reinterpret_cast<u8*>(this) = byte;
}

void StatusByte::printStatus(){
    printf("byte: %u,error : %d, ready : %d, busy : %d\n",
              *reinterpret_cast<uchar*>(this),error,ready,busy);
}

StatusByte HDD::getStatus() const{
    return inb(_basePort + 7);
}

bool HDD::isThereADisk(){
    return inb(_basePort + 7) != 0xFF;
}

void HDD::init(){
    readlba(0,MBR,1);
    memcpy(table,MBR+0x1be,4*sizeof(PartitionTableEntry));
}

HDD::HDD(u8 bus,bool master) : _master(master),active(false),table{},MBR{}
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


void HDD::writelba (u32 LBA , const void* data, u32 nbsector){
    assert(nbsector > 0 && nbsector < 512 && data != nullptr);
    activate();
    outb (_basePort + 6, 0x40 |((!_master)<<4));
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, 0/*(LBA >> 32) & 0xFF*/);
    outb (_basePort + 5, 0/*(LBA >> 40) & 0xFF*/);
    outb (_basePort + 2, nbsector & 0xFF);
    outb (_basePort + 3, LBA & 0xFF);
    outb (_basePort + 4, (LBA >> 8) & 0xFF);
    outb (_basePort + 5, (LBA >> 16) & 0xFF);
    outb (_basePort + 7,0x34);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
    for(u64 i = 0 ; i < nbsector*256 ; ++ i ){
        if (i % 256 == 0){
            while(!getStatus().isReadyOrFailed()){}
            assert(getStatus().isOk());
        }
        WAIT(10);
        outw(_basePort,reinterpret_cast<const ushort*>(data)[i]);
    }



    outb (_basePort + 7,0xE7); // cache flush
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
}


void HDD::readlba (u32 LBA, void * data, u32 nbsector)const{
    //printf("HDD reading sector %u\n",LBA);
    readaddr(u64(LBA)*u64(512),data,u64(nbsector)*u64(512));

}

void HDD::writeaddr (u64 addr , const void* data, size_t size){
    assert(size < 512*512);
    static uchar buffer[512];
    u32 LBA = addr/512L;
    size_t offset = addr %512L;
    if (offset != 0){
        readlba(LBA,buffer,1);
        size_t toCopy = min(512-offset,size);
        memcpy(buffer+offset,data,toCopy);
        writelba(LBA,buffer,1);
        if(toCopy == size) return; // TODO not tested from this point
        ++LBA;
        data = reinterpret_cast<const uchar*>(data) + toCopy;
        size -= (512-offset);
    }
    u32 toWriteSectors = size /512L;
    if(toWriteSectors > 0){
        writelba(LBA,data,toWriteSectors);
        size -= toWriteSectors * 512;
        data = reinterpret_cast<const uchar*>(data) + 512 * toWriteSectors;
        LBA+=toWriteSectors;
    }
    if (size > 0){
        readlba(LBA,buffer,1);
        memcpy(buffer,data,size);
        writelba(LBA,buffer,1);
    }

}
void HDD::readaddr (u64 addr, void * data, size_t size) const{
    assert(size > 0 && size < (512*512) && addr < (u64(1) << (32+9)));
    activate();
    u32 LBA = addr/512;
    u32 offset = addr %512;
    u64 endLBA = (addr + size + 511)/512;
    u64 nbsector = endLBA - LBA;
    u64 count = 0; // in words not bytes !!!
    outb (_basePort + 6, 0x40 |((!_master)<<4));
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, 0/*(LBA >> 32) & 0xFF*/);
    outb (_basePort + 5, 0/*(LBA >> 40) & 0xFF*/);
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
        count ++;
    }
    if(offset == 1){
        u16 d = inw(_basePort);
        ++count;
        *reinterpret_cast<u8*>(data) = d >> 8;
        data = reinterpret_cast<u8*>(data) +1;
        size--;
    }
    for( ;size > 1; size -=2 ){
        if(count % 256 == 0) {
            while(!getStatus().isReadyOrFailed()){}
            assert(getStatus().isOk());
        }
        ++count;
        *reinterpret_cast<u16*>(data) = inw(_basePort);
        data = reinterpret_cast<u16*>(data) +1;
    }
    //size +=2;
    if (size == 1){
        ++count;
        u16 d = inw(_basePort);
        *reinterpret_cast<u8*>(data) = u8(d);
    }
    while(count < nbsector * 256){ //finish to read useless data.
        ++count;
        inw(_basePort);
    }
}

PartitionTableEntry HDD::operator[](u8 i) const{
    assert(i>0 && i <=4);
    return table[i-1];
}

const PartitionTableEntry*  HDD::partWithPred(std::function<bool(const Partition&)> pred){
    for(int i =0 ; i < 4 ; ++i){
        if(table[i].size == 0 || table[i].systemID == 0) continue;
        if(pred(Partition(this,table[i]))) return &table[i];
    }
    return nullptr;
}

