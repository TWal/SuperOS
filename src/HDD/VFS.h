#ifndef VFS_H
#define VFS_H

#include "FileSystem.h"
#include <map>

namespace HDD {

namespace VFS {

class Directory;

/// @brief VFS filesystem
class FS : public FileSystem {
    public:
        FS(FileSystem* fsroot);
        virtual std::unique_ptr<::HDD::Directory> getRoot();
        ///Mount `fs` at directory `dir`
        void mount(std::unique_ptr<Directory> dir, FileSystem* fs);
        ///Unmount directory `dir`
        void umount(std::unique_ptr<Directory> dir);

    private:
        Directory* toMounted(u32 inode, u32 dev);
        Directory* fromMounted(u32 inode, u32 dev);
        friend class Directory;
        u32 _nextDev;
        FileSystem* _root;
        std::map<std::pair<u32, u32>, Directory*> _mountedDirs;
        std::map<std::pair<u32, u32>, Directory*> _reverseMountedDirs;
};

class File {
    public:
        File(FS* fs, ::HDD::File* impl, u32 dev);
        void i_getStats(stat* buf) const;
        FileType i_getType() const;

    protected:
        FS* _fs;
        ::HDD::File* _impl;
        u32 _dev;
};

class RegularFile : public ::HDD::RegularFile, public File {
    public:
        RegularFile(FS* fs, std::unique_ptr<::HDD::RegularFile> impl, u32 dev);
        virtual void getStats(stat* buf) const;
        virtual void resize(size_t size);
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual size_t getSize() const;
    protected:
        std::unique_ptr<::HDD::RegularFile> _impl;
};

class Directory : public ::HDD::Directory, public File {
    public:
        Directory(FS* fs, std::unique_ptr<::HDD::Directory> impl, u32 dev);
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
    protected:
        friend class FS;
        std::unique_ptr<::HDD::Directory> _impl;
};

class BlockDevice : public ::HDD::BlockDevice, public File {
    public:
        BlockDevice(FS* fs, std::unique_ptr<::HDD::BlockDevice> impl, u32 dev);
        virtual void getStats(stat* buf) const;
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual size_t getSize() const;
    protected:
        friend class FS;
        std::unique_ptr<::HDD::BlockDevice> _impl;
};

class CharacterDevice : public ::HDD::CharacterDevice, public File {
    public:
        CharacterDevice(FS* fs, std::unique_ptr<::HDD::CharacterDevice> impl, u32 dev);
        virtual void getStats(stat* buf) const;
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        virtual size_t tell() const;
        virtual size_t seek(i64 count, mod mode);
    protected:
        friend class FS;
        std::unique_ptr<::HDD::CharacterDevice> _impl;
};

extern FS* vfs;


} //end of namespace VFS

} //end of namespace HDD

#endif

