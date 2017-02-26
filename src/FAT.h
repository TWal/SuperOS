#ifndef FAT_H
#define FAT_H

#include "FileSystem.h"
#include <string>
#include <map>

namespace fat {

    struct MBR {
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


    static_assert(sizeof(MBR)==90,"Wrong size for FATMBR");

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
        bool directory : 1;
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
        uint getCluster(){return (uint(clusterHigh) << 16)+ clusterLow;}
        void setCluster(uint cluster)
            {clusterLow = cluster & 0xFFFF; clusterHigh = cluster >> 16;}
        std::string getName();
    }__attribute((packed));

    static_assert(sizeof(DirectoryEntry) == 32, "Wrong size for DirectoryEntry");


    struct LongFileName{
        uchar order;
        ushort first5[5];
        uchar attribute; // should be 0x0F
        uchar longEntryType;
        uchar checksum;
        ushort second6[6];
        uchar zero[2];
        ushort third2[2];
        std::string getName();
    }__attribute((packed));
    static_assert(sizeof(LongFileName) == 32, "Wrong size for LongFileName");

    class FS;
    class Directory;

    class File : public virtual ::File{
    protected :
        Directory* _parent;
        FS* _fs;
        uint _cluster;
        uint _clusterSize;
        ulint _size;
    public :
        ulint getSize();
        bool isInRAM(){return false;}
        void* getData(){return nullptr;}
        void writeaddr (ulint addr,const void * data, uint size);
        void readaddr (ulint addr, void * data, uint size);
        void writelba (ulint LBA , const void* data, uint nbsector);
        void readlba (ulint LBA, void * data, uint nbsector);

        explicit File(FS* fs, uint cluster,ulint size, Directory* parent = nullptr);
        void setName(const std::string& name);
        virtual ::Directory * getParent(); // null => root directory

    };


    /*class DATAFile : public virtual File, public virtual ::DATAFile {
    public :
        using File::getSize;
        using File::isInRAM;
        using File::getData;
        using File::writeaddr;
        using File::writelba;
        using File::readaddr;
        using File::readlba;
        DATAFile(FS* fs, uint cluster, ulint size,Directory* parent = nullptr);
        };*/

    class Directory : public File, public ::Directory{
        std::map<std::string,fat::File*> _content;
        std::string fusion(const std::map<uchar,LongFileName>& longName);
        bool _loaded;
    public :
        Directory(FS* fs, uint cluster, ulint size,Directory* parent = nullptr);
        void load(); // load directory in RAM.
        std::vector<std::string> getFilesName ();
        File * operator[](const std::string& name);
    };

    class FS : public FileSystem {
        friend class File;
        friend class DATAFile;
        friend class Directory;
        MBR _fmbr;
    public:
        explicit FS (Partition* part);
        uint getFATEntry(uint cluster);
        uint clusterToLBA(uint cluster,uint offset);
        uint nbRemainingCluster(uint cluster);
        ::Directory* getRoot();
        Directory* getRootFat();
    };

}

#endif
