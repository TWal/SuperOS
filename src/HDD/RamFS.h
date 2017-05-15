#ifndef RAMFS_H
#define RAMFS_H

#include "FileSystem.h"
#include <map>

namespace HDD {

namespace RamFS {

class File;
class RegularFile;
class Directory;
class BlockDevice;
class CharacterDevice;

class FS : public FileSystem {
    public:
        FS();
        virtual std::unique_ptr<::HDD::Directory> getRoot();
        /// Create a fresh new file
        RegularFile* getNewFile(u16 uid, u16 gid, u16 mode);
        /// Create a fresh new directory
        Directory* getNewDirectory(u16 uid, u16 gid, u16 mode);
        /// Create a fresh new block device
        BlockDevice* getNewBlockDevice(HDDBytes* impl, u16 uid, u16 gid, u16 mode);
        /// Create a fresh new character device
        CharacterDevice* getNewCharacterDevice(Stream* impl, u16 uid, u16 gij, u16 mode);

    protected:
        Directory* _root;
        u32 _nextInode;
};

class File {
    public:
        File(u32 ino, u16 mode, u16 uid, u16 gid);
        virtual ~File();
        void i_getStats(stat* buf) const;
        void link();
        void unlink();
        virtual size_t size() const = 0;

    protected:
        friend class RegularFile;
        friend class Directory;
        u32 _ino;
        u16 _mode;
        u16 _nlink;
        u16 _uid;
        u16 _gid;
};

class RegularFile : public ::HDD::RegularFile, public File {
    public:
        RegularFile(u32 ino, u16 mode, u16 uid, u16 gid);
        virtual void getStats(stat* buf) const;
        virtual void resize(size_t size);
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual size_t getSize() const;
        virtual size_t size() const;

    protected:
        std::string _content;
};

class Directory : public ::HDD::Directory, public File {
    public:
        Directory(FS* fs, u32 ino, u16 mode, u16 uid, u16 gid);
        virtual void getStats(stat* buf) const;
        virtual std::unique_ptr<::HDD::File> operator[](const std::string& name);
        virtual size_t size() const;

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

    protected:
        FS* _fs;
        std::map<std::string, ::HDD::File*> _entries;
        struct DirIterator {
            std::map<std::string, ::HDD::File*>::iterator it;
            dirent res;
        };
};

class BlockDevice : public ::HDD::BlockDevice, public File {
    public:
        BlockDevice(HDDBytes* impl, u32 ino, u16 mode, u16 uid, u16 gid);
        virtual void getStats(stat* buf) const;
        virtual size_t size() const;
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual size_t getSize() const;
    protected:
        HDDBytes* _impl;
};

class CharacterDevice : public ::HDD::CharacterDevice, public File {
    public:
        CharacterDevice(Stream* impl, u32 ino, u16 mode, u16 uid, u16 gid);
        virtual void getStats(stat* buf) const;
        virtual size_t size() const;
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count) const;
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        virtual size_t tell() const;
        virtual size_t seek(i64 count, mod mode);
    protected:
        Stream* _impl;
};


} // end of namespace RamFS

} // end of namespace HDD

#endif

