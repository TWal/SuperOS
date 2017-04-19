#include "VFS.h"
#include <string.h>

namespace VFS {

FS::FS(FileSystem* fsroot) :
    FileSystem(nullptr), _nextDev(1), _root(fsroot) {}


::Directory* FS::getRoot() {
    return vgetRoot();
}

Directory* FS::vgetRoot() {
    return new Directory(_root->getRoot(), 0, this);
}

void FS::mount(Directory* dir, FileSystem* fs) {
    stat st;
    dir->getStats(&st);
    ::Directory* root = fs->getRoot();
    _mountedDirs.insert(std::make_pair(std::make_pair(dir->_dev, dir->getInode()), new Directory(root, _nextDev, this)));
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

File::File(::File* impl, u32 dev) : _dev(dev), _impl(impl) {}

void File::writeaddr(u64 addr, const void* data, size_t size) {
    _impl->writeaddr(addr, data, size);
}

void File::readaddr(u64 addr, void* data, size_t size) const {
    _impl->readaddr(addr, data, size);
}

size_t File::getSize() const {
    return _impl->getSize();
}


void File::resize(size_t size) {
    _impl->resize(size);
}

void File::link() {
    _impl->link();
}

void File::unlink() {
    _impl->unlink();
}

void File::getStats(stat* buf) const {
    _impl->getStats(buf);
    buf->st_dev = _dev;
}

Directory::Directory(::Directory* impl, u32 dev, FS* fs) : File(impl, dev), _impl(impl), _fs(fs) {
}


::File* Directory::operator[](const std::string& name) {
    return get(name);
}

File* Directory::get(const std::string& name) {
    Directory* d;
    if(std::string("..") == name && (d = _fs->fromMounted(getInode(), _dev)) != nullptr) {
        return d->get("..");
    }

    ::File* res = (*_impl)[name];
    if(res == nullptr) {
        return nullptr;
    }
    if((d = _fs->toMounted(res->getInode(), _dev)) != nullptr) {
        return d;
    }
    if(res->getType() == FileType::File) {
        return new File(res, _dev);
    } else if(res->getType() == FileType::Directory) {
        return new Directory(res->dir(), _dev, _fs);
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

void Directory::addEntry(const std::string& name, ::File* file) {
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

void Directory::deleteDir() {
    _impl->deleteDir();
}


}

