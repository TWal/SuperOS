#ifndef EXT2_H
#define EXT2_H

#include "../utility.h"
#include "FileSystem.h"

namespace HDD {

namespace Ext2 {

/// @brief Ext2's superblock structure
struct SuperBlock {
    u32 inodes_count;      ///< Count of inodes in the filesystem
    u32 blocks_count;      ///< Count of blocks in the filesystem
    u32 r_blocks_count;    ///< Count of the number of reserved blocks
    u32 free_blocks_count; ///< Count of the number of free blocks
    u32 free_inodes_count; ///< Count of the number of free inodes
    u32 first_data_block;  ///< The first block which contains data
    u32 log_block_size;    ///< Indicator of the block size
    i32 log_frag_size;     ///< Indicator of the size of the fragments
    u32 blocks_per_group;  ///< Count of the number of blocks in each block group
    u32 frags_per_group;   ///< Count of the number of fragments in each block group
    u32 inodes_per_group;  ///< Count of the number of inodes in each block group
    u32 mtime;             ///< The time that the filesystem was last mounted
    u32 wtime;             ///< The time that the filesystem was last written to
    u16 mnt_count;         ///< The number of times the file system has been mounted
    u16 max_mnt_count;     ///< The number of times the file system can be mounted
    u16 magic;             ///< Magic number indicating ex2fs
    u16 state;             ///< Flags indicating the current state of the filesystem
    u16 errors;            ///< Flags indicating the procedures for error reporting
    u16 pad;               ///< padding
    u32 lastcheck;         ///< The time that the filesystem was last checked
    u32 checkinterval;     ///< The maximum time permissible between checks
    u32 creator_os;        ///< Indicator of which OS created the filesystem
    u32 rev_level;         ///< The revision level of the filesystem
    u16 def_resuid;
    u16 def_resgid;

    u32 first_ino;         ///< First non-reserved inode
    u16 inode_size;        ///< Size of inode structure
    u16 block_group_nr;    ///< Block group # of this superblock
    u32 feature_compat;    ///< Compatible feature set
    u32 feature_incompat;  ///< Incompatible feature set
    u32 feature_ro_compat; ///< Readonly-compatible feature set
    u32 uuid[4];           ///< 128-bit uuid for volume
    char volume_name[16];  ///< Volume name
    char last_mounted[64]; ///< Directory where last mounted
    u32 algo_bitmap;       ///< For compression
    u32 reserved[205];     ///< Padding to 1024 bytes
} __attribute__((packed));

static_assert(sizeof(SuperBlock) == 1024, "SuperBlock has the wrong size");

inline bool CheckUUID(u32 UUID[4],const Partition& part){
    SuperBlock sb;
    part.readaddr(1024,&sb,sizeof(SuperBlock));
    printf("UUID : ");
    for(int i = 0 ; i < 4 ; ++i){
        printf("%8x-",(sb.uuid)[i]);
    }printf("\n");
    return sb.uuid[0] == UUID[0] && sb.uuid[1] == UUID[1] &&
        sb.uuid[2] == UUID[2] && sb.uuid[3] == UUID[3];
}


enum FileSystemState {
    FSS_CLEAN = 1,
    FSS_ERROR = 2
};

enum ErrorHandlingMethod {
    EHM_IGNORE = 1,
    EHM_READONLY = 2,
    EHM_PANIC = 3
};

/// @brief Ext2's block group descriptor structure
struct BlockGroupDescriptor {
    u32 block_bitmap;      ///< The address of the block containing the block bitmap for this group
    u32 inode_bitmap;      ///< The address of the block containing the inode bitmap for this group
    u32 inode_table;       ///< The address of the block containing the inode table for this group
    u16 free_blocks_count; ///< The count of free blocks in this group
    u16 free_inodes_count; ///< The count of free inodes in this group
    u16 used_dirs_count;   ///< The number inodes in this group which are directories
    u16 pad;
    u32 reserved[3];
} __attribute__((packed));

static_assert(sizeof(BlockGroupDescriptor) == 32, "BlockGroupDescriptor has the wrong size");

/// @brief Ext2's inode structure
struct InodeData {
    u16 mode;        ///< File mode
    u16 uid;         ///< Owner Uid
    u32 size;        ///< Size in bytes
    u32 atime;       ///< Access time
    u32 ctime;       ///< Creation time
    u32 mtime;       ///< Modification time
    u32 dtime;       ///< Deletion Time
    u16 gid;         ///< Group Id
    u16 links_count; ///< Links count
    u32 blocks;      ///< Blocks count (block of 512 byte!!)
    u32 flags;       ///< File flags
    u32 reserved1;   ///< OS dependent
    u32 block[15];   ///< Pointers to blocks
    u32 version;     ///< File version (for NFS)
    u32 file_acl;    ///< File ACL
    u32 dir_acl;     ///< Directory ACL
    u32 faddr;       ///< Fragment address
    u8 frag;         ///< Fragment number
    u8 fsize;        ///< Fragment size
    u16 pad1;
    u32 reserved2[2];
    /// Get the block count measured in ext2's block size
    u32 getBlockCount(const SuperBlock& sb) const;
    /// Update `blocks` to increment `getBlockCount` by `diff`
    void incrBlockCount(int diff, const SuperBlock& sb);
    size_t getSize() const;
    void setSize(size_t size);
} __attribute__((packed));

static_assert(sizeof(InodeData) == 128, "InodeData has the wrong size");

/// @brief Ext2's directory entry structure
struct DirectoryEntry {
    u32 inode;    ///< Address if inode
    u16 rec_len;  ///< Length of this record
    u8 name_len;  ///< Length of file name
    u8 type;      ///< Type of the file
    char name[0]; ///< The file name
} __attribute__((packed));

enum DirectoryFileType {
    FT_UNKNOWN = 0,  ///< Unknown File Type
    FT_REG_FILE = 1, ///< Regular File
    FT_DIR = 2,      ///< Directory File
    FT_CHRDEV = 3,   ///< Character Device
    FT_BLKDEV = 4,   ///< Block Device
    FT_FIFO = 5,     ///< Buffer File
    FT_SOCK = 6,     ///< Socket File
    FT_SYMLINK = 7   ///< Symbolic Link
};


static_assert(sizeof(DirectoryEntry) == 8, "DirectoryEntry has the wrong size");

class Inode;
class RegularFile;
class Directory;

/// @brief Ext2 filesystem
class FS : public FileSystem {
    public:
        explicit FS(Partition* part);
        virtual std::unique_ptr<::HDD::Directory> getRoot();
        /// Create a fresh new file
        RegularFile* getNewFile(u16 uid, u16 gid, u16 mode);
        /// Create a fresh new directory
        Directory* getNewDirectory(u16 uid, u16 gid, u16 mode);

        /// Get an `InodeData` from an inode number
        void getInodeData(u32 inode, InodeData* res) const;
        /// Write an `InodeData` from an inode number
        void writeInodeData(u32 inode, const InodeData* data);
        /// Find and allocate a block near an inode
        u32 getNewBlock(u32 nearInode);
        /// Free a block
        void freeBlock(u32 block);
        /// Find and allocate an inode
        u32 getNewInode(bool isDirectory);
        /// Free an inode
        void freeInode(u32 inode, bool isDirectory);

    private:
        friend class Inode;
        friend class RegularFile;
        friend class Directory;
        /// Load the superblock
        void _loadSuperBlock();
        /// Write the superblock
        void _writeSuperBlock();
        /// Load the block group descriptor array
        void _loadBlockGroupDescriptor();
        /// Write the block group descriptor array
        void _writeBlockGroupDescriptor();
        Partition* _part;
        SuperBlock _sb;
        BlockGroupDescriptor* _bgd;
        u64 _blockSize;
        u64 _nbBgd;
};

/// @brief An inode in the Ext2 filesystem
class Inode {
    public:
        Inode(FS* fs, u32 inode, InodeData data);
        /// internal readaddr
        void i_readaddr(u64 addr, void* data, size_t size) const;
        /// internal writeaddr
        void i_writeaddr(u64 addr, const void* data, size_t size);
        /// internal resize
        void i_resize(size_t size);
        /// internal getStats
        void i_getStats(stat* buf) const;
        /// internal getSize
        size_t i_getSize() const;
        /// increment the link count of the inode
        void link();
        /// decrement the link count of the inode, delete it if the link count becomes zero
        void unlink();

    protected:
        FS* _fs;
        u32 _inode;
        InodeData _data;

    private:
        struct ReadRecArgs {
            u64 addr;
            u8* data;
            size_t size;
            u64 indirectSize[4];
            u32* blocks[3];
        };
        void readrec(ReadRecArgs& args, int level, u32 blockId) const;

        struct WriteRecArgs : public ReadRecArgs {
            u64 indirectBlock[4];
            u32 blockNum;
        };
        bool writerec(WriteRecArgs& args, int level, u32* blockId);

        struct ResizeRecArgs {
            size_t sizeAfter;
            size_t sizeBefore;
            u64 indirectSize[4];
            u32* blocks[3];
        };
        bool resizerec(ResizeRecArgs& args, int level, u32 blockId);
};

/// @brief Ext2 regular file
class RegularFile : public ::HDD::RegularFile, public Inode {
    public:
        RegularFile(FS* fs, u32 inode, InodeData data);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void resize(size_t size);
        virtual void getStats(stat* buf) const;
        virtual size_t getSize() const;
};

/// @brief Ext2 directory
class Directory : public ::HDD::Directory, public Inode {
    public :
        Directory(FS* fs, u32 inode, InodeData data);
        virtual void getStats(stat* buf) const;
        virtual std::unique_ptr<::HDD::File> operator[](const std::string& name);
        virtual void* open();
        virtual dirent* read(void* d);
        virtual long int tell(void* d);
        virtual void seek(void* d, long int loc);
        virtual void close(void* d);

        virtual std::unique_ptr<::HDD::File> addEntry(const std::string& name, u16 uid, u16 gid, u16 mode);
        virtual void addEntry(const std::string& name, ::HDD::File* file);
        virtual void removeFile(const std::string& name);
        virtual void removeDirectory(const std::string& name);
        virtual void removeEntry(const std::string& name);

        /// initializes the directory with '.'
        void init();
        /// remove all the entries in the directory (including '.')
        void deleteDir();

    protected:

        struct DirIterator {
            DirectoryEntry entry;
            u32 pos;
            dirent res;
        };
};

} //end of namespace Ext2

} //end of namespace HDD

#endif

