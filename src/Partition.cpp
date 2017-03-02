#include "Partition.h"
#include "HardDrive.h"


Partition::Partition (HDD*HDD,PartitionTableEntry descriptor):_descriptor(descriptor),_HDD(HDD){
    assert(HDD != nullptr);
}

size_t Partition::getSize(){
    return (_descriptor.endLBA - _descriptor.begLBA)*512;
}

void Partition::writelba (u32 LBA , const void* data, u32 nbsector){
    assert (LBA + nbsector < _descriptor.endLBA - _descriptor.begLBA);
    _HDD->writelba(LBA + _descriptor.begLBA,data,nbsector);
}
void Partition::readlba (u32 LBA, void * data, u32 nbsector){
    assert (LBA + nbsector < _descriptor.endLBA - _descriptor.begLBA);
    _HDD->readlba(LBA + _descriptor.begLBA,data,nbsector);
}
void Partition::writeaddr (uptr addr , const void* data, size_t size){
    assert (addr + size < getSize());
    _HDD->writeaddr(addr + _descriptor.begLBA*512,data,size);
}
void Partition::readaddr (uptr addr, void * data, size_t size){
    assert (addr + size < getSize());
    _HDD->readaddr(addr + _descriptor.begLBA*512,data,size);
}
