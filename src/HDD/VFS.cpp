#include "VFS.h"
#include <string.h>

namespace HDD {

namespace VFS {

FS::FS(FileSystem* fsroot) :
    _nextDev(1), _root(fsroot) {}


std::unique_ptr<::HDD::Directory> FS::getRoot() {
    return std::unique_ptr<::HDD::Directory>(new Directory(this, _root->getRoot(), 0));
}

void FS::mount(std::unique_ptr<Directory> dir, FileSystem* fs) {
    stat st;
    dir->getStats(&st);
    std::unique_ptr<::HDD::Directory> root = fs->getRoot();
    root.dontDelete();
    u32 rootInode = root->getInode();
    _mountedDirs.insert(std::make_pair(std::make_pair(dir->_dev, dir->getInode()), new Directory(this, std::move(root), _nextDev)));
    _reverseMountedDirs.insert(std::make_pair(std::make_pair(_nextDev, rootInode), dir.release()));
    _nextDev += 1;
}

void FS::umount(std::unique_ptr<Directory> dir) {
    (void) dir;
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


std::unique_ptr<::HDD::File> FS::convert(std::unique_ptr<::HDD::File>&& file, u32 dev) {
    if(file->getType() == FileType::RegularFile) {
        return std::unique_ptr<::HDD::File>(new RegularFile(this, std::lifted_static_cast<::HDD::RegularFile>(std::move(file)), dev));
    } else if(file->getType() == FileType::Directory) {
        return std::unique_ptr<::HDD::File>(new Directory(this, std::lifted_static_cast<::HDD::Directory>(std::move(file)), dev));
    } else if(file->getType() == FileType::BlockDevice) {
        return std::unique_ptr<::HDD::File>(new BlockDevice(this, std::lifted_static_cast<::HDD::BlockDevice>(std::move(file)), dev));
    } else if(file->getType() == FileType::CharacterDevice) {
        return std::unique_ptr<::HDD::File>(new CharacterDevice(this, std::lifted_static_cast<::HDD::CharacterDevice>(std::move(file)), dev));
    } else {
        bsod("HDD::VFS::FS::convert: oh noes! me dunno how 2 handle dat file type");
    }
}


/*   _____ _ _
    |  ___(_) | ___
    | |_  | | |/ _ \
    |  _| | | |  __/
    |_|   |_|_|\___|
*/

File::File(FS* fs, ::HDD::File* impl, u32 dev) :
    _fs(fs), _impl(std::move(impl)), _dev(dev) { }

void File::i_getStats(stat* buf) const {
    _impl->getStats(buf);
    buf->st_dev = _dev;
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

RegularFile::RegularFile(FS* fs, std::unique_ptr<::HDD::RegularFile> impl, u32 dev) :
    VFS::File(fs, impl.get(), dev), _impl(std::move(impl)) {}

void RegularFile::getStats(stat* buf) const {
    i_getStats(buf);
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

Directory::Directory(FS* fs, std::unique_ptr<::HDD::Directory> impl, u32 dev) :
    VFS::File(fs, impl.get(), dev), _impl(std::move(impl)) {}

void Directory::getStats(stat* buf) const {
    i_getStats(buf);
}

std::unique_ptr<::HDD::File> Directory::operator[](const std::string& name) {
    Directory* d;
    if(std::string("..") == name && (d = _fs->fromMounted(getInode(), _dev)) != nullptr) {
        return (*d)[".."];
    }

    std::unique_ptr<::HDD::File> res = (*_impl)[name];
    if(res.get() == nullptr) {
        return nullptr;
    }
    if((d = _fs->toMounted(res->getInode(), _dev)) != nullptr) {
        std::unique_ptr<::HDD::File> res(d);
        res.dontDelete();
        return res;
    }
    return _fs->convert(std::move(res), _dev);
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

std::unique_ptr<::HDD::File> Directory::addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) {
    return _fs->convert(_impl->addEntry(name, uid, gid, mode), _dev);
}

void Directory::addEntry(const std::string& name, ::HDD::File* file) {
    switch(file->getType()) {
        case FileType::RegularFile:
            _impl->addEntry(name, static_cast<RegularFile*>(file)->_impl.get());
            break;
        case FileType::Directory:
            _impl->addEntry(name, static_cast<Directory*>(file)->_impl.get());
            break;
        case FileType::BlockDevice:
            _impl->addEntry(name, static_cast<BlockDevice*>(file)->_impl.get());
            break;
        case FileType::CharacterDevice:
            _impl->addEntry(name, static_cast<CharacterDevice*>(file)->_impl.get());
            break;
    }
}

void Directory::removeDirectory(const std::string& name) {
    _impl->removeDirectory(name);
}

void Directory::removeEntry(const std::string& name) {
    _impl->removeEntry(name);
}

/*  ____  _            _    ____             _
   | __ )| | ___   ___| | _|  _ \  _____   _(_) ___ ___
   |  _ \| |/ _ \ / __| |/ / | | |/ _ \ \ / / |/ __/ _ \
   | |_) | | (_) | (__|   <| |_| |  __/\ V /| | (_|  __/
   |____/|_|\___/ \___|_|\_\____/ \___| \_/ |_|\___\___|
*/

BlockDevice::BlockDevice(FS* fs, std::unique_ptr<::HDD::BlockDevice> impl, u32 dev) :
    VFS::File(fs, impl.get(), dev), _impl(std::move(impl)) {}

void BlockDevice::getStats(stat* buf) const {
    i_getStats(buf);
}

void BlockDevice::writeaddr(u64 addr, const void* data, size_t size) {
    _impl->writeaddr(addr, data, size);
}

void BlockDevice::readaddr(u64 addr, void* data, size_t size) const {
    _impl->readaddr(addr, data, size);
}

size_t BlockDevice::getSize() const {
    return _impl->getSize();
}


/*   ____ _                          _            ____             _
    / ___| |__   __ _ _ __ __ _  ___| |_ ___ _ __|  _ \  _____   _(_) ___ ___
   | |   | '_ \ / _` | '__/ _` |/ __| __/ _ \ '__| | | |/ _ \ \ / / |/ __/ _ \
   | |___| | | | (_| | | | (_| | (__| ||  __/ |  | |_| |  __/\ V /| | (_|  __/
    \____|_| |_|\__,_|_|  \__,_|\___|\__\___|_|  |____/ \___| \_/ |_|\___\___|
*/

CharacterDevice::CharacterDevice(FS* fs, std::unique_ptr<::HDD::CharacterDevice> impl, u32 dev) :
    VFS::File(fs, impl.get(), dev), _impl(std::move(impl)) {}

void CharacterDevice::getStats(stat* buf) const {
    i_getStats(buf);
}

u64 CharacterDevice::getMask() const {
    return _impl->getMask();
}

size_t CharacterDevice::read(void* buf, size_t count) {
    return _impl->read(buf, count);
}

bool CharacterDevice::eof() const {
    return _impl->eof();
}

size_t CharacterDevice::write(const void* buf, size_t count) {
    return _impl->write(buf, count);
}

size_t CharacterDevice::tell() const {
    return _impl->tell();
}

size_t CharacterDevice::seek(i64 count, mod mode) {
    return _impl->seek(count, mode);
}


FS* vfs;
} //end of namespace VFS

} //end of namespace HDD

