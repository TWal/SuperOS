#ifndef FAT_H
#define FAT_H

#include "FileSystem.h"
#include <string>
#include <map>

namespace fat {

    struct MBR {
        u8 startCode [3];
        char OEMIdent [8];
        u16 BytesPerSector;
        u8 SectorPerCluster;
        u16 nbofReservedSectors;
        u8 nbofFAT;
        u16 nbofDirEntries;
        u16 smallSize;
        u8 mediaDescriptor;
        u8 legacyGarbage [6];
        u32 nbHidden;
        u32 largeSize;
        u32 FATSize;
        u16 Flags;
        u16 FATVersion;
        u32 RootCluster;
        u16 FSInfoSector;
        u16 BackupSector;
        u8 reserved[12];
        u8 DriveNumber;
        u8 reserved2;
        u8 Signature;
        u32 SerialNumber;
        char Label[11];
        char Ident [8];

    }__attribute__((packed));


    static_assert(sizeof(MBR)==90,"Wrong size for FATMBR");

    struct Date {
        u8 year : 7; // since 1980
        u8 month : 4;
        u8 day : 5;
    }__attribute__((packed));
    struct Hour {
        u8 hour : 5;
        u8 min : 6;
        u8 dblSec : 5; // 2* seconds
    }__attribute__((packed));


    struct DirectoryEntry {
        char shortName[11];
        bool readOnly : 1;
        bool hidden : 1;
        bool system : 1;
        bool volume_id : 1;
        bool directory : 1;
        bool archive : 1;
        u8 nothing : 2;
        u8 reserved;
        u8 creationDuration;
        Hour creationHour;
        Date creationDate;
        Date lastAccessed;
        u16 clusterHigh;
        Hour lastModifHour;
        Date lastModifDate;
        u16 clusterLow;
        u32 size;
        u32 getCluster(){return (u32(clusterHigh) << 16)+ clusterLow;}
        void setCluster(u32 cluster)
            {clusterLow = cluster & 0xFFFF; clusterHigh = cluster >> 16;}
        std::string getName();
    }__attribute__((packed));

    static_assert(sizeof(DirectoryEntry) == 32, "Wrong size for DirectoryEntry");


    struct LongFileName{
        u8 order;
        u16 first5[5];
        u8 attribute; // should be 0x0F
        u8 longEntryType;
        u8 checksum;
        u16 second6[6];
        u8 zero[2];
        u16 third2[2];
        std::string getName();
    }__attribute__((packed));
    static_assert(sizeof(LongFileName) == 32, "Wrong size for LongFileName");

    class FS;
    class Directory;

    class File : public virtual ::File{
    protected :
        Directory* _parent;
        FS* _fs;
        u32 _cluster;
        u32 _clusterSize;
        u32 _size;
    public :
        virtual size_t getSize() const;
        virtual void writeaddr (u64 addr,const void * data, size_t size);
        virtual void readaddr (u64 addr, void * data, size_t size) const;
        virtual void writelba (u32 LBA , const void* data, u32 nbsector);
        virtual void readlba (u32 LBA, void * data, u32 nbsector) const;
        virtual void resize(size_t size) {
            bsod("fat::File::resize not implemented!");
        }

        explicit File(FS* fs, u32 cluster,size_t size, Directory* parent = nullptr);
        void setName(const std::string& name);
        virtual ::Directory * getParent(); // null => root directory

    };



    class Directory : public File, public ::Directory{
        std::map<std::string,fat::File*> _content;
        std::string fusion(const std::map<u8,LongFileName>& longName);
        bool _loaded;
    public :
        Directory(FS* fs, u32 cluster, size_t size,Directory* parent = nullptr);
        void load(); // load directory in RAM.
        //std::vector<std::string> getFilesName ();
        File * operator[](const std::string& name);
        virtual void* open() {
            bsod("fat::Directory::open not implemented");
        }
        virtual dirent* read(void* d) {
            bsod("fat::Directory::read not implemented");
        }
        virtual long int tell(void* d) {
            bsod("fat::Directory::tell not implemented");
        }
        virtual void seek(void* d, long int loc) {
            bsod("fat::Directory::seek not implemented");
        }
        virtual void close(void* d) {
            bsod("fat::Directory::close not implemented");
        }
        virtual File* addFile(const std::string& name, u16 mode, u16 uid, u16 gid) {
            bsod("fat::Directory::addFile not implemented");
        }
        virtual void removeFile(const std::string& name) {
            bsod("fat::Directory::removeFile not implemented");
        }

    };

    class FS : public FileSystem {
        friend class File;
        friend class Directory;
        MBR _fmbr;
    public:
        explicit FS (Partition* part);
        u32 getFATEntry(u32 cluster)const;
        u32 clusterToLBA(u32 cluster,u32 offset)const;
        u32 nbRemainingCluster(u32 cluster)const;
        virtual ::Directory* getRoot();
        Directory* getRootFat();
    };


    inline bool CheckUUID(u32 UUID,const Partition& part){
        MBR buffer;
        part.readaddr(0,&buffer,sizeof(MBR));
        printf("Found UUID : %x\n",buffer.SerialNumber);
        return buffer.SerialNumber == UUID;
    }

}

#endif
