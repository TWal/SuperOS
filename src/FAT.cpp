#include <string.h>
#include "FAT.h"
#include "FrameBuffer.h"
#include "globals.h"

FATFile::FATFile(FATFS* fs, uint cluster) : _fs(fs),_cluster(cluster){
    _clusterSize = _fs->_fmbr.SectorPerCluster;

}
ulint FATFile::getSize(){
    return _size;
}
void FATFile::writeaddr (ulint addr,const void * data, uint size){
    assert (size <= 512);// will take care of larger write later
    uint clusterOffset = addr / (512*_clusterSize);
    uint offset = addr - clusterOffset *512 * _clusterSize;
    uint LBA = _fs->clusterToLBA(_cluster,clusterOffset);
    _fs->_part->writeaddr(LBA*512 + offset,data,size);
}
void FATFile::readaddr (ulint addr, void * data, uint size){
    assert (size <= 512);// will take care of larger read later
    uint clusterOffset = addr / (512*_clusterSize);
    uint offset = addr - clusterOffset *512 * _clusterSize;
    uint LBA = _fs->clusterToLBA(_cluster,clusterOffset);
    _fs->_part->readaddr(LBA*512 + offset,data,size);
}
void FATFile::writelba (ulint LBA , const void* data, uint nbsector){
    assert (nbsector == 1);// will take care of larger write later
    uint clusterOffset = LBA / _clusterSize;
    uint offset = LBA - clusterOffset * _clusterSize;
    uint LBAo = _fs->clusterToLBA(_cluster,clusterOffset);
    _fs->_part->writelba(LBAo+ offset + offset,data,nbsector);
}
void FATFile::readlba (ulint LBA, void * data, uint nbsector){
    assert (nbsector == 1);// will take care of larger read later
    uint clusterOffset = LBA / _clusterSize;
    uint offset = LBA - clusterOffset * _clusterSize;
    uint LBAo = _fs->clusterToLBA(_cluster,clusterOffset);
    _fs->_part->readlba(LBAo+ offset + offset,data,nbsector);
}


FATFS::FATFS (Partition* part) : FileSystem(part){
    //assert(_fmbr.BytesPerSector == 512); // TODO generic sector size.
    _part->readaddr(0,&_fmbr,sizeof(_fmbr));
    char name [12] = {};
    memcpy(name,_fmbr.Label,11);
    fb.printf("Opening FAT32 filesystem : %s; with FATSize of %u with reserved sectors %d\n"
              ,name,_fmbr.FATSize,_fmbr.nbofReservedSectors);

}

uint FATFS::getFATEntry(uint cluster){
    uint res;
    _part->readaddr(_fmbr.nbofReservedSectors *512 + cluster *4,&res,4);
    return res;
}
uint FATFS::clusterToLBA(uint cluster,uint offset){
    cluster &= 0x0FFFFFFF;
    assert (cluster != 0x0FFFFFF7);
    if (cluster > 0x0FFFFFF7) return -1;
    if(offset == 0){
        return _fmbr.nbofReservedSectors + _fmbr.nbofFAT*_fmbr.FATSize-2 + cluster;
        //-2 is because the first 2 entries of the FAT are unused
    }
    else {
        return clusterToLBA(getFATEntry(cluster),offset-1);
    }
}
