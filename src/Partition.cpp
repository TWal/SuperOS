#include "Partition.h"
#include "HardDrive.h"


Partition::Partition (HDD*HDD,PartitionTableEntry descriptor):_descriptor(descriptor),_HDD(HDD){
    assert(HDD != nullptr);
}

ulint Partition::getSize(){
    return (_descriptor.endLBA - _descriptor.begLBA)*512;
}

void Partition::writelba (ulint LBA , const void* data, uint nbsector){
    assert (LBA + nbsector < _descriptor.endLBA - _descriptor.begLBA);
    _HDD->writelba(LBA + _descriptor.begLBA,data,nbsector);
}
void Partition::readlba (ulint LBA, void * data, uint nbsector){
    assert (LBA + nbsector < _descriptor.endLBA - _descriptor.begLBA);
    _HDD->readlba(LBA + _descriptor.begLBA,data,nbsector);
}
void Partition::writeaddr (ulint addr , const void* data, uint size){
    assert (addr + size < getSize());
    _HDD->writeaddr(addr + _descriptor.begLBA*512,data,size);
}
void Partition::readaddr (ulint addr, void * data, uint size){
    assert (addr + size < getSize());
    _HDD->readaddr(addr + _descriptor.begLBA*512,data,size);
}
