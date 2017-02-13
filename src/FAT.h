#ifndef FAT_H
#define FAT_H

#include "FileSystem.h"

struct FATMBR {
    uchar startCode [3];
    char OEMIdent [8];
    ushort BytesPerSector;
    uchar SectorPerCluster;
    ushort nbofReservedSectors;
    uchar nbofFAT;
    ushort nbofDirEntries;
    ushort smallSize;
    uchar mediaDescriptor;
    uchar legacyGarbage [6];
    uint nbHidden;
    uint largeSize;
    uint FATSize;
    ushort Flags;
    ushort FATVersion;
    uint RootCluster;
    ushort FSInfoSector;
    ushort BackupSector;
    uchar reserved[12];
    uchar DriveNumber;
    uchar reserved2;
    uchar Signature;
    uint SerialNumber;
    char Label[11];
    char Ident [8];

}__attribute((packed));


static_assert(sizeof(FATMBR)==90,"Wrong size for FATMBR");

struct Date {
    uchar year : 7; // since 1980
    uchar month : 4;
    uchar day : 5;
}__attribute((packed));
struct Hour {
    uchar hour : 5;
    uchar min : 6;
    uchar dblSec : 5; // 2* seconds
}__attribute((packed));


struct DirectoryEntry {
    char shortName[11];
    bool readOnly : 1;
    bool hidden : 1;
    bool system : 1;
    bool volume_id : 1;
    bool direcotry : 1;
    bool archive : 1;
    uchar nothing :2;
    uchar reserved;
    uchar creationDuration;
    Hour creationHour;
    Date creationDate;
    Date lastAccessed;
    ushort clusterHigh;
    Hour lastModifHour;
    Date lastModifDate;
    ushort clusterLow;
    uint size;
}__attribute((packed));

static_assert(sizeof(DirectoryEntry) == 32, "Wrong size for Directory");

class FATFS;

class FATFile : public File {
    FATFS* _fs;
    uint _cluster;
    uint _clusterSize;
    ulint _size;
public :
    FATFile(FATFS* fs,uint cluster);
    ulint getSize();
    bool isInRAM(){return false;}
    void* getData(){return nullptr;}
    void writeaddr (ulint addr,const void * data, uint size);
    void readaddr (ulint addr, void * data, uint size);
    void writelba (ulint LBA , const void* data, uint nbsector);
    void readlba (ulint LBA, void * data, uint nbsector);


};

class FATFS : public FileSystem {
    friend class FATFile;
    FATMBR _fmbr;
public:
    explicit FATFS (Partition* part);
    uint getFATEntry(uint cluster);
    uint clusterToLBA(uint cluster,uint offset);
};

#endif
