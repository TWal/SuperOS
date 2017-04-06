#ifndef EXT2_H
#define EXT2_H

#include "../utility.h"
#include "FileSystem.h"

namespace Ext2 {


struct SuperBlock {
    u32 inodes_count; //Count of inodes in the filesystem
    u32 blocks_count; //Count of blocks in the filesystem
    u32 r_blocks_count; //Count of the number of reserved blocks
    u32 free_blocks_count; //Count of the number of free blocks
    u32 free_inodes_count; //Count of the number of free inodes
    u32 first_data_block; //The first block which contains data
    u32 log_block_size; //Indicator of the block size
    i32 log_frag_size; //Indicator of the size of the fragments
    u32 blocks_per_group; //Count of the number of blocks in each block group
    u32 frags_per_group; //Count of the number of fragments in each block group
    u32 inodes_per_group; //Count of the number of inodes in each block group
    u32 mtime; //The time that the filesystem was last mounted
    u32 wtime; //The time that the filesystem was last written to
    u16 mnt_count; //The number of times the file system has been mounted
    u16 max_mnt_count; //The number of times the file system can be mounted
    u16 magic; //Magic number indicating ex2fs
    u16 state; //Flags indicating the current state of the filesystem
    u16 errors; //Flags indicating the procedures for error reporting
    u16 pad; //padding
    u32 lastcheck; //The time that the filesystem was last checked
    u32 checkinterval; //The maximum time permissible between checks
    u32 creator_os; //Indicator of which OS created the filesystem
    u32 rev_level; //The revision level of the filesystem
    u16 def_resuid;
    u16 def_resgid;
    //
    u32 first_ino; //First non-reserved inode 
    u16 inode_size; //size of inode structure 
    u16 block_group_nr; //block group # of this superblock 
    u32 feature_compat; //compatible feature set 
    u32 feature_incompat; //incompatible feature set 
    u32 feature_ro_compat; //readonly-compatible feature set 
    u32 uuid[4]; //128-bit uuid for volume 
    char volume_name[16]; //volume name 
    char last_mounted[64]; //directory where last mounted 
    u32 algo_bitmap; //For compression 
    u32 reserved[205]; //padding to 1024 bytes
} __attribute__((packed));

static_assert(sizeof(SuperBlock) == 1024, "SuperBlock has the wrong size");

enum FileSystemState {
    FSS_CLEAN = 1,
    FSS_ERROR = 2
};

enum ErrorHandlingMethod {
    EHM_IGNORE = 1,
    EHM_READONLY = 2,
    EHM_PANIC = 3
};

struct BlockGroupDescriptor {
    u32 block_bitmap; //The address of the block containing the block bitmap for this group
    u32 inode_bitmap; //The address of the block containing the inode bitmap for this group
    u32 inode_table; //The address of the block containing the inode table for this group
    u16 free_blocks_count; //The count of free blocks in this group
    u16 free_inodes_count; //The count of free inodes in this group
    u16 used_dirs_count; //The number inodes in this group which are directories
    u16 pad; //padding
    u32 reserved[3]; //padding
} __attribute__((packed));

static_assert(sizeof(BlockGroupDescriptor) == 32, "BlockGroupDescriptor has the wrong size");

struct InodeData {
    u16 mode; //File mode
    u16 uid; //Owner Uid
    u32 size; //Size in bytes
    u32 atime; //Access time
    u32 ctime; //Creation time
    u32 mtime; //Modification time
    u32 dtime; //Deletion Time
    u16 gid; //Group Id
    u16 links_count; //Links count
    u32 blocks; //Blocks count (block of 512 byte!!)
    u32 flags; //File flags
    u32 reserved1; //OS dependent
    u32 block[15]; //Pointers to blocks
    u32 version; //File version (for NFS)
    u32 file_acl; //File ACL
    u32 dir_acl; //Directory ACL
    u32 faddr; //Fragment address
    u8 frag; //Fragment number
    u8 fsize; //Fragment size
    u16 pad1;
    u32 reserved2[2];
    u32 getBlockCount(const SuperBlock& sb) const;
    void incrBlockCount(int diff, const SuperBlock& sb);
} __attribute__((packed));

static_assert(sizeof(InodeData) == 128, "InodeData has the wrong size");

enum FileMode {
    FM_IFMT = 0xF000, //format mask
    FM_IFSOCK = 0xA000, //socket
    FM_IFLNK = 0xC000, //symbolic link
    FM_IFREG = 0x8000, //regular file
    FM_IFBLK = 0x6000, //block device
    FM_IFDIR = 0x4000, //directory
    FM_IFCHR = 0x2000, //character device
    FM_IFIFO = 0x1000, //fifo

    FM_ISUID = 0x0800, //SUID
    FM_ISGID = 0x0400, //SGID
    FM_ISVTX = 0x0200, //sticky bit

    FM_IRWXU = 0x01C0, //user mask
    FM_IRUSR = 0x0100, //read
    FM_IWUSR = 0x0080, //write
    FM_IXUSR = 0x0040, //execute

    FM_IRWXG = 0x0038, //group mask
    FM_IRGRP = 0x0020, //read
    FM_IWGRP = 0x0010, //write
    FM_IXGRP = 0x0008, //execute

    FM_IRWXO = 0x0007, //other mask
    FM_IROTH = 0x0004, //read
    FM_IWOTH = 0x0002, //write
    FM_IXOTH = 0x0001  //execute
};

struct DirectoryEntry {
    u32 inode;    //address if inode
    u16 rec_len;  //length of this record
    u8 name_len;  //length of file name
    u8 type;
    char name[0]; //the file name
} __attribute__((packed));

static_assert(sizeof(DirectoryEntry) == 8, "DirectoryEntry has the wrong size");

class FS : public FileSystem {
    public:
        explicit FS(Partition* part);
        virtual ::Directory* getRoot();
        void getInodeData(u32 inode, InodeData* res) const;
        void writeInodeData(u32 inode, const InodeData* data);
        u32 getNewBlock(u32 nearInode);
        void freeBlock(u32 block);

    private:
        friend class File;
        void _loadSuperBlock();
        void _writeSuperBlock();
        void _loadBlockGroupDescriptor();
        void _writeBlockGroupDescriptor();
        SuperBlock _sb;
        BlockGroupDescriptor* _bgd;
        uint _blockSize;
        uint _nbBgd;
};

class File : public virtual ::File {
    public:
        File(u32 inode, InodeData data, FS* fs);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void resize(size_t size);
        virtual size_t getSize() const;

    protected:
        struct ReadRecArgs {
            u64 addr;
            u8* data;
            size_t size;
            u64 indirectSize[4];
            u32* blocks[3];
        };
        void _readrec(ReadRecArgs& args, int level, u32 blockId) const;

        struct WriteRecArgs : public ReadRecArgs {
            u64 indirectBlock[4];
            u32 blockNum;
        };
        bool _writerec(WriteRecArgs& args, int level, u32* blockId);

        struct ResizeRecArgs {
            size_t sizeAfter;
            size_t sizeBefore;
            u64 indirectSize[4];
            u32* blocks[3];
        };
        bool _resizerec(ResizeRecArgs& args, int level, u32 blockId);


        u32 _inode;
        InodeData _data;
        FS* _fs;
};

struct dirent {
    u32 d_ino;
    char d_name[256];
};

class Directory : public virtual File, public virtual ::Directory {
    public :
        class iterator {
            public:
                iterator& operator++();
                dirent operator*();
                bool operator==(const iterator& other);
                bool operator!=(const iterator& other);

            private:
                friend class Directory;
                iterator(Directory* father, u32 pos);
                iterator(Directory* father);
                Directory* _father;
                DirectoryEntry _entry;
                u32 _pos;
                char _name[256];
        };
        Directory(u32 inode, InodeData data, FS* fs);
        virtual std::vector<std::string> getFilesName();
        virtual Ext2::File* operator[](const std::string& name);
        iterator begin();
        iterator end();
    private:
        friend class iterator;
};

}

#endif

