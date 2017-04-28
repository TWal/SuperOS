#include "RamFS.h"

namespace HDD {

namespace RamFS {

/*  _____ ____
   |  ___/ ___|
   | |_  \___ \
   |  _|  ___) |
   |_|   |____/
*/

FS::FS() : _root(nullptr), _nextInode(0) {
    _root = getNewDirectory(0, 0, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

::HDD::Directory* FS::FS::getRoot() {
    return _root;
}

RegularFile* FS::getNewFile(u16 uid, u16 gid, u16 mode) {
    return new RegularFile(this, _nextInode++, mode, uid, gid);
}

Directory* FS::getNewDirectory(u16 uid, u16 gid, u16 mode) {
    Directory* res = new Directory(this, _nextInode++, mode, uid, gid);
    res->addEntry(".", res);
    return res;
}

/*  _____ _ _
   |  ___(_) | ___
   | |_  | | |/ _ \
   |  _| | | |  __/
   |_|   |_|_|\___|
*/


File::File(FS* fs, u32 ino, u16 mode, u16 uid, u16 gid) :
    _fs(fs), _ino(ino), _mode(mode), _nlink(0), _uid(uid), _gid(gid) {}

void File::i_getStats(stat* buf) const {
    buf->st_ino = _ino;
    buf->st_mode = _mode;
    buf->st_nlink = _nlink;
    buf->st_uid = _uid;
    buf->st_gid = _gid;
    size_t sz = size();
    buf->st_size = sz;
    buf->st_blksize = sz/512;
    buf->st_blocks = sz/512;
}

void File::link() {
    _nlink += 1;
}

void File::unlink() {
    _nlink -= 1;
}

/*  ____                  _            _____ _ _
   |  _ \ ___  __ _ _   _| | __ _ _ __|  ___(_) | ___
   | |_) / _ \/ _` | | | | |/ _` | '__| |_  | | |/ _ \
   |  _ <  __/ (_| | |_| | | (_| | |  |  _| | | |  __/
   |_| \_\___|\__, |\__,_|_|\__,_|_|  |_|   |_|_|\___|
              |___/
*/

RegularFile::RegularFile(FS* fs, u32 ino, u16 mode, u16 uid, u16 gid) :
    RamFS::File(fs, ino, mode, uid, gid) {}

void RegularFile::getStats(stat* buf) const {
    RamFS::File::i_getStats(buf);
}

void RegularFile::resize(size_t size) {
    _content.resize(size);
}

void RegularFile::writeaddr(u64 addr, const void* data, size_t size) {
    if(_content.size() < addr + size) {
        _content.resize(addr+size);
    }
    memcpy((char*)_content.c_str() + addr, data, size);
}

void RegularFile::readaddr(u64 addr, void* data, size_t size) const {
    assert(addr+size <= _content.size());
    memcpy(data, _content.c_str() + addr, size);
}

size_t RegularFile::getSize() const {
    return _content.size();
}

size_t RegularFile::size() const {
    return _content.size();
}

/*  ____  _               _
   |  _ \(_)_ __ ___  ___| |_ ___  _ __ _   _
   | | | | | '__/ _ \/ __| __/ _ \| '__| | | |
   | |_| | | | |  __/ (__| || (_) | |  | |_| |
   |____/|_|_|  \___|\___|\__\___/|_|   \__, |
                                        |___/
*/

Directory::Directory(FS* fs, u32 ino, u16 mode, u16 uid, u16 gid) : RamFS::File(fs, ino, mode, uid, gid) {
}

void Directory::getStats(stat* buf) const {
    RamFS::File::i_getStats(buf);
}

::HDD::File* Directory::operator[](const std::string& name) {
    auto it = _entries.find(name);
    if(it == _entries.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

size_t Directory::size() const {
    return 0; //MEH
}

void* Directory::open() {
    DirIterator* res = new DirIterator;
    res->it = _entries.begin();
}

dirent* Directory::read(void* d) {
    DirIterator* di = (DirIterator*)d;
    if(di->it == _entries.end()) {
        return nullptr;
    }

    strncpy(di->res.d_name, di->it->first.c_str(), 256);
    di->res.d_ino = di->it->second->getInode();;
    ++(di->it);
    return &di->res;
}

long int Directory::tell(void* d) {
    return std::distance(_entries.begin(), ((DirIterator*)d)->it);
}

void Directory::seek(void* d, long int loc) {
    DirIterator* di = (DirIterator*)d;
    di->it = _entries.begin();
    std::advance(di->it, loc);
}

void Directory::close(void* d) {
    delete (DirIterator*)d;
}

void Directory::addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) {
    ::HDD::File* f = nullptr;
    if(S_ISREG(mode)) {
        f = _fs->getNewFile(uid, gid, mode);
    } else if(S_ISDIR(mode)) {
        f = _fs->getNewDirectory(uid, gid, mode);
    } else {
        bsod("HDD::RamFS::Directory::addEntry: me dunno wat to do wiz mode %u\n", mode);
    }
    addEntry(name, f);
}

void Directory::addEntry(const std::string& name, ::HDD::File* file) {
    if(file->getType() == FileType::RegularFile) {
        static_cast<RegularFile*>(file)->link();
    } else if(file->getType() == FileType::Directory) {
        static_cast<Directory*>(file)->link();
    }
    _entries.insert(std::make_pair(name, file));
    if(file->getType() == FileType::Directory && std::string("..") != name) {
        dynamic_cast<Directory*>(file)->addEntry("..", this);
    }
}

void Directory::removeFile(const std::string& name) {
    removeEntry(name);
}

void Directory::removeDirectory(const std::string& name) {
    removeEntry(name);
}

void Directory::removeEntry(const std::string& name) {
    assert(_entries.erase(name));
}

} // end of namespace RamFS

} // end of namespace HDD

