#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Partition.h"
#include <vector>
#include <string>
#include "../Streams/Stream.h"
#include <memory>
#include <unistd.h>

namespace HDD {

enum class FileType {
    RegularFile, Directory, BlockDevice, CharacterDevice
};

/**
    @brief Represents a file / directory / ... in the filesystem
*/
class File {
    public:
        virtual ~File(){}
        /// Returns the type of the file
        virtual FileType getType() const = 0;

        /// get status of the file
        virtual void getStats(stat* buf) const = 0;
        /// get inode number using getStats
        virtual u32 getInode() const;
};

/**
    @brief Represents a regular in the filesystem
*/
class RegularFile : public File, public HDDBytes {
    public:
        virtual FileType getType() const;
        /// Resize the file
        virtual void resize(size_t size) = 0;
        /// A regular file is appendable.
        bool appendable(){return true;}
};

/**
    @brief Represents a directory in the filesystem
*/
class Directory : public File {
    public:
        virtual FileType getType() const;
        ///Get a file in the directory. Returns nullptr when it does not exists
        virtual std::unique_ptr<File> operator[](const std::string& name) = 0;
        ///Get the file at `path`
        std::unique_ptr<File> resolvePath(const std::string& path);

        virtual void* open() = 0; ///< Like opendir
        virtual dirent* read(void* d) = 0; ///< Like readdir
        virtual long int tell(void* d) = 0; ///< Like telldir
        virtual void seek(void* d, long int loc) = 0; ///< Like seekdir
        virtual void close(void* d) = 0; ///< Like closedir

        ///Add an entry in the directory, creating the file/directory. Returns the file created.
        virtual std::unique_ptr<File> addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) = 0;
        ///Add an existing entry in the directory (it may be used to create hard links)
        virtual void addEntry(const std::string& name, File* file) = 0;
        ///Remove and delete directory in a directory
        virtual void removeDirectory(const std::string& name) = 0;
        ///Remove (and delete) an entry. This is the reciprocal of addEntry(const std::string&, File*)
        virtual void removeEntry(const std::string& name) = 0;

        ///Check if the directory is empty
        virtual bool isEmpty();
};

/**
    @brief Represents a block device in the filesystem
*/
class BlockDevice : public File, public HDDBytes {
    public:
        virtual FileType getType() const;
};

/**
    @brief Represents a character device in the filesystem
*/
class CharacterDevice : public File, public Stream {
    public:
        virtual FileType getType() const;
};

/**
    @brief Represents a filesystem

    This may be some filesystem implementation (FAT, Ext2), some special filesystem (procfs, devfs),
    or the implementation of a VFS
*/
class FileSystem {
    public:
        virtual std::unique_ptr<Directory> getRoot() = 0;
};

} //end of namespace HDD

#endif
