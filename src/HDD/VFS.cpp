#include "VFS.h"
#include <string.h>
#include "../IO/FrameBuffer.h"

namespace HDD {

namespace VFS {

FS::FS(FileSystem* fsroot) :
    _nextDev(1), _root(fsroot) {}


::HDD::Directory* FS::getRoot() {
    return new Directory(this, _root->getRoot(), 0);
}

void FS::mount(Directory* dir, FileSystem* fs) {
    stat st;
    dir->getStats(&st);
    ::HDD::Directory* root = fs->getRoot();
    _mountedDirs.insert(std::make_pair(std::make_pair(dir->_dev, dir->getInode()), new Directory(this, root, _nextDev)));
    _reverseMountedDirs.insert(std::make_pair(std::make_pair(_nextDev, root->getInode()), dir));
}

void FS::umount(Directory* dir) {
    bsod("VFS::FS::umount not implemented yet\n");
}

Directory* FS::toMounted(u32 inode, u32 dev) {
    auto it = _mountedDirs.find(std::make_pair(dev, inode));
    if(it == _mountedDirs.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

Directory* FS::fromMounted(u32 inode, u32 dev) {
    auto it = _reverseMountedDirs.find(std::make_pair(dev, inode));
    if(it == _reverseMountedDirs.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}


/*   _____ _ _
    |  ___(_) | ___
    | |_  | | |/ _ \
    |  _| | | |  __/
    |_|   |_|_|\___|
*/

File::File(FS* fs, ::HDD::File* impl, u32 dev) :
    _fs(fs), _impl(impl), _dev(dev) {}

void File::i_getStats(stat* buf) const {
    _impl->getStats(buf);
}

FileType File::i_getType() const {
    return _impl->getType();
}



/*   ____                  _            _____ _ _
    |  _ \ ___  __ _ _   _| | __ _ _ __|  ___(_) | ___
    | |_) / _ \/ _` | | | | |/ _` | '__| |_  | | |/ _ \
    |  _ <  __/ (_| | |_| | | (_| | |  |  _| | | |  __/
    |_| \_\___|\__, |\__,_|_|\__,_|_|  |_|   |_|_|\___|
               |___/
*/

RegularFile::RegularFile(FS* fs, ::HDD::RegularFile* impl, u32 dev) :
    VFS::File(fs, impl, dev), _impl(impl) {}

void RegularFile::getStats(stat* buf) const {
    _impl->getStats(buf);
}

void RegularFile::resize(size_t size) {
    _impl->resize(size);
}

void RegularFile::writeaddr(u64 addr, const void* data, size_t size) {
    _impl->writeaddr(addr, data, size);
}

void RegularFile::readaddr(u64 addr, void* data, size_t size) const {
    _impl->readaddr(addr, data, size);
}

size_t RegularFile::getSize() const {
    return _impl->getSize();
}


/*   ____  _               _
    |  _ \(_)_ __ ___  ___| |_ ___  _ __ _   _
    | | | | | '__/ _ \/ __| __/ _ \| '__| | | |
    | |_| | | | |  __/ (__| || (_) | |  | |_| |
    |____/|_|_|  \___|\___|\__\___/|_|   \__, |
                                         |___/
*/

Directory::Directory(FS* fs, ::HDD::Directory* impl, u32 dev) :
    VFS::File(fs, impl, dev), _impl(impl) {}

void Directory::getStats(stat* buf) const {
    i_getStats(buf);
}

::HDD::File* Directory::operator[](const std::string& name) {
    Directory* d;
    if(std::string("..") == name && (d = _fs->fromMounted(getInode(), _dev)) != nullptr) {
        return (*d)[".."];
    }

    ::HDD::File* res = (*_impl)[name];
    if(res == nullptr) {
        return nullptr;
    }
    if((d = _fs->toMounted(res->getInode(), _dev)) != nullptr) {
        return d;
    }
    if(res->getType() == FileType::RegularFile) {
        return new RegularFile(_fs, static_cast<::HDD::RegularFile*>(res), _dev);
    } else if(res->getType() == FileType::Directory) {
        ::HDD::Directory* d = static_cast<::HDD::Directory*>(res);
        return new Directory(_fs, d, _dev);
    } else {
        bsod("VFS::Directory::get: oh noes! me dunno how 2 handle dat file type");
    }
}

void* Directory::open() {
    return _impl->open();
}

dirent* Directory::read(void* d) {
    dirent* res = _impl->read(d);

    if(res == nullptr) {
        return nullptr;
    }

    Directory* dir;
    if(res->d_name[0] == '.' && res->d_name[1] == '.' && res->d_name[2] == '\0' &&
       (dir = _fs->fromMounted(getInode(), _dev)) != nullptr) {
        res->d_ino = (*dir)[".."]->getInode();
    } else if((dir = _fs->toMounted(res->d_ino, _dev)) != nullptr) {
        res->d_ino = dir->getInode();
    }

    return res;
}

long int Directory::tell(void* d) {
    return _impl->tell(d);
}

void Directory::seek(void* d, long int loc) {
    _impl->seek(d, loc);
}

void Directory::close(void* d) {
    _impl->close(d);
}

void Directory::addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) {
    _impl->addEntry(name, uid, gid, mode);
}

void Directory::addEntry(const std::string& name, ::HDD::File* file) {
    _impl->addEntry(name, file);
}

void Directory::removeFile(const std::string& name) {
    _impl->removeFile(name);
}

void Directory::removeDirectory(const std::string& name) {
    _impl->removeDirectory(name);
}

void Directory::removeEntry(const std::string& name) {
    _impl->removeEntry(name);
}

} //end of namespace VFS

} //end of namespace HDD

