#ifndef VFS_H
#define VFS_H

#include "FileSystem.h"
#include <map>

namespace VFS {

class Directory;

/// @brief VFS filesystem
class FS : public FileSystem {
    public:
        FS(FileSystem* fsroot);
        virtual ::Directory* getRoot();
        Directory* vgetRoot();
        ///Mount `fs` at directory `dir`
        void mount(Directory* dir, FileSystem* fs);
        ///Unmount directory `dir`
        void umount(Directory* dir);

    private:
        Directory* toMounted(u32 inode, u32 dev);
        Directory* fromMounted(u32 inode, u32 dev);
        friend class Directory;
        u32 _nextDev;
        FileSystem* _root;
        std::map<std::pair<u32, u32>, Directory*> _mountedDirs;
        std::map<std::pair<u32, u32>, Directory*> _reverseMountedDirs;
};

/// @brief VFS file
class File : public virtual ::File {
    public:
        File(::File* impl, u32 dev);
        virtual void writeaddr(u64 addr, const void* data, size_t size);
        virtual void readaddr(u64 addr, void* data, size_t size) const;
        virtual size_t getSize() const;

        virtual void resize(size_t size);
        virtual void link();
        virtual void unlink();
        virtual void getStats(stat* buf) const;

        virtual FileType vgetType() const {
            return FileType::File;
        }
        virtual Directory* vdir() {
            return nullptr;
        }
    protected:
        u32 _dev;
    private:
        ::File* _impl;
};

/// @brief VFS directory
class Directory : public File, public ::Directory {
    public :
        Directory(::Directory* impl, u32 dev, FS* fs);
        virtual ::File* operator[](const std::string& name);
        virtual File* get(const std::string& name);

        virtual void* open();
        virtual dirent* read(void* d);
        virtual long int tell(void* d);
        virtual void seek(void* d, long int loc);
        virtual void close(void* d);

        virtual void addEntry(const std::string& name, u16 uid, u16 gid, u16 mode);
        virtual void addEntry(const std::string& name, ::File* file);
        virtual void removeFile(const std::string& name);
        virtual void removeDirectory(const std::string& name);
        virtual void removeEntry(const std::string& name);

        virtual void deleteDir();


        virtual FileType vgetType() const {
            return FileType::Directory;
        }
        virtual Directory* vdir() {
            return this;
        }
    private:
        friend class FS;
        ::Directory* _impl;
        FS* _fs;
};

}

#endif

