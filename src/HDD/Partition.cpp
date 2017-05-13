#include "Partition.h"
#include "HardDrive.h"

namespace HDD {

Partition::Partition (HDD*HDD,PartitionTableEntry descriptor):_descriptor(descriptor),_HDD(HDD){
    assert(HDD != nullptr);
}

size_t Partition::getSize() const {
    return size_t(_descriptor.size)*512ull;
}

void Partition::writelba (u32 LBA, const void* data, u32 nbsector) {
    assert(LBA + nbsector < _descriptor.size);
    _HDD->writelba(LBA + _descriptor.begLBA, data, nbsector);
}
void Partition::readlba(u32 LBA, void* data, u32 nbsector) const {
    assert(LBA + nbsector < _descriptor.size);
    _HDD->readlba(LBA + _descriptor.begLBA, data, nbsector);
}
void Partition::writeaddr(u64 addr, const void* data, size_t size) {
    assert(addr + size < getSize());
    _HDD->writeaddr(addr + u64(_descriptor.begLBA)*512ull, data, size);
}
void Partition::readaddr(u64 addr, void * data, size_t size) const {
    assert(addr + size < getSize());
    _HDD->readaddr(addr + u64(_descriptor.begLBA)*512ull, data, size);
}

} //end of namespace HDD

