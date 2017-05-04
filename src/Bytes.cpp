#include "Bytes.h"

void HDDBytes::writelba(u32 LBA , const void* data, u32 nbsector) {
    writeaddr(LBA*512, data, nbsector*512);
}

void HDDBytes::readlba(u32 LBA, void* data, u32 nbsector) const {
    readaddr(LBA*512, data, nbsector*512);
}

