#include "Ext2.h"
#include "../IO/FrameBuffer.h"

namespace Ext2 {

u32 InodeData::getBlockCount(const SuperBlock& sb) const {
    return blocks/(2<<sb.log_block_size);
}

void InodeData::incrBlockCount(int diff, const SuperBlock& sb) {
    blocks += diff*(2<<sb.log_block_size);
}

size_t InodeData::getSize() const {
    return size;
    return (size_t)size | (((size_t)dir_acl) << 32);
}

void InodeData::setSize(size_t sz) {
    size = sz & (((size_t)1<<32) - 1);
    dir_acl = (sz >> 32) & (((size_t)1<<32) - 1);
}


FS::FS(Partition* part) : FileSystem(part) {
    _bgd = nullptr;
    _loadSuperBlock();
    assert(_sb.magic == 0xef53);
    assert(_sb.log_block_size == (u32)_sb.log_frag_size);
    _loadBlockGroupDescriptor();
}

::Directory* FS::getRoot() {
    InodeData dat;
    getInodeData(2, &dat);
    return new Directory(2, dat, this);
}

::File* FS::getNewFile(u16 uid, u16 gid, u16 mode) {
    assert(S_ISREG(mode)); //TODO handle this better
    u32 inode = getNewInode(false);
    InodeData dat;
    memset(&dat, 0, sizeof(InodeData));
    dat.uid = uid;
    dat.gid = gid;
    dat.mode = mode;
    writeInodeData(inode, &dat);
    return new File(inode, dat, this);
}

::Directory* FS::getNewDirectory(u16 uid, u16 gid, u16 mode) {
    assert(S_ISDIR(mode)); //TODO handle this better
    u32 inode = getNewInode(true);
    //update bgd
    _writeBlockGroupDescriptor();
    //write inode
    InodeData dat;
    memset(&dat, 0, sizeof(InodeData));
    dat.uid = uid;
    dat.gid = gid;
    dat.mode = mode;
    writeInodeData(inode, &dat);
    Directory* d = new Directory(inode, dat, this);
    d->init();
    return d;
}


void FS::getInodeData(u32 inode, InodeData* res) const {
    uint blockGroup = (inode-1) / _sb.inodes_per_group;
    uint index      = (inode-1) % _sb.inodes_per_group;
    uint inodeTable = _bgd[blockGroup].inode_table;
    _part->readaddr(inodeTable*_blockSize + index*_sb.inode_size, res, sizeof(InodeData));
}

void FS::writeInodeData(u32 inode, const InodeData* data) {
    uint blockGroup = (inode-1) / _sb.inodes_per_group;
    uint index      = (inode-1) % _sb.inodes_per_group;
    uint inodeTable = _bgd[blockGroup].inode_table;
    _part->writeaddr(inodeTable*_blockSize + index*_sb.inode_size, data, sizeof(InodeData));
}

u32 FS::getNewBlock(u32 nearInode) {
    //TODO: some bitmap caching to speed up writing
    uint blockGroup = (nearInode-1) / _sb.inodes_per_group;
    u8* bitmap = (u8*)malloc(_blockSize);
    for(uint i = 0; i < _nbBgd; ++i) {
        uint bgd = (i+blockGroup)%_nbBgd;
        if(_bgd[bgd].free_blocks_count == 0) continue;
        _part->readaddr(_bgd[bgd].block_bitmap*_blockSize, bitmap, _blockSize);
        for(uint j = 0; j < _blockSize; ++j) {
            if((u8)~bitmap[j] != 0) {
                uint pos = __builtin_ctz(~bitmap[j]);
                assert((bitmap[j] & (1 << pos)) == 0);
                bitmap[j] |= (1 << pos);
                _part->writeaddr(_bgd[bgd].block_bitmap*_blockSize, bitmap, _blockSize);
                _bgd[bgd].free_blocks_count -= 1;
                _sb.free_blocks_count -= 1;
                _writeBlockGroupDescriptor(); //TODO do this less often
                _writeSuperBlock();
                free(bitmap);
                return _sb.blocks_per_group*bgd + 8*sizeof(u8)*j + pos;
            }
        }
    }
    free(bitmap);
    bsod("No space on disk :-("); //TODO handle this properly
}

void FS::freeBlock(u32 block) {
    u32 blockGroup = block / _sb.blocks_per_group;
    u32 blockIndex = block % _sb.blocks_per_group;
    u8* bitmap = (u8*)malloc(_blockSize);

    _part->readaddr(_bgd[blockGroup].block_bitmap*_blockSize, bitmap, _blockSize);

    u32 bitmapInd = blockIndex / (8*sizeof(u8));
    u32 byteInd   = blockIndex % (8*sizeof(u8));
    assert((bitmap[bitmapInd] & (1 << byteInd)) != 0);
    bitmap[bitmapInd] &= ~(1 << byteInd);
    assert((bitmap[bitmapInd] & (1 << byteInd)) == 0);

    _part->writeaddr(_bgd[blockGroup].block_bitmap*_blockSize, bitmap, _blockSize);

    _bgd[blockGroup].free_blocks_count += 1;
    _sb.free_blocks_count += 1;

    _writeBlockGroupDescriptor(); //TODO do this less often
    _writeSuperBlock();
    free(bitmap);
}

u32 FS::getNewInode(bool isDirectory) {
    u8* bitmap = (u8*)malloc(_blockSize);
    for(uint i = 0; i < _nbBgd; ++i) {
        if(_bgd[i].free_inodes_count == 0) continue;
        _part->readaddr(_bgd[i].inode_bitmap*_blockSize, bitmap, _blockSize);
        for(uint j = 0; j < _blockSize; ++j) {
            if((u8)~bitmap[j] != 0) {
                uint pos = __builtin_ctz(~bitmap[j]);
                assert((bitmap[j] & (1 << pos)) == 0);
                bitmap[j] |= (1 << pos);
                _part->writeaddr(_bgd[i].inode_bitmap*_blockSize, bitmap, _blockSize);
                _bgd[i].free_inodes_count -= 1;
                _sb.free_inodes_count -= 1;
                if(isDirectory) {
                    _bgd[i].used_dirs_count += 1;
                }
                _writeBlockGroupDescriptor();
                _writeSuperBlock();
                free(bitmap);
                return _sb.inodes_per_group*i + 8*sizeof(u8)*j + pos + 1;
            }
        }
    }
    free(bitmap);
    bsod("No more inodes on disk :-("); //TODO handle this properly
}

void FS::freeInode(u32 inode, bool isDirectory) {
    u32 inodeGroup = (inode - 1) / _sb.inodes_per_group;
    u32 inodeIndex = (inode - 1) % _sb.inodes_per_group;
    u8* bitmap = (u8*)malloc(_blockSize);

    _part->readaddr(_bgd[inodeGroup].inode_bitmap*_blockSize, bitmap, _blockSize);

    u32 bitmapInd = inodeIndex / (8*sizeof(u8));
    u32 byteInd   = inodeIndex % (8*sizeof(u8));
    assert((bitmap[bitmapInd] & (1 << byteInd)) != 0);
    bitmap[bitmapInd] &= ~(1 << byteInd);
    assert((bitmap[bitmapInd] & (1 << byteInd)) == 0);

    _part->writeaddr(_bgd[inodeGroup].inode_bitmap*_blockSize, bitmap, _blockSize);

    _bgd[inodeGroup].free_inodes_count += 1;
    _sb.free_inodes_count += 1;
    if(isDirectory) {
        _bgd[inodeGroup].used_dirs_count -= 1;
    }

    _writeBlockGroupDescriptor(); //TODO do this less often
    _writeSuperBlock();
    free(bitmap);
}


void FS::_loadSuperBlock() {
    _part->readaddr(1024, &_sb, sizeof(SuperBlock));
    if(_sb.rev_level == 0) {
        _sb.inode_size = 128;
    }
    _blockSize = 1024 << _sb.log_block_size;
    _nbBgd = ((_sb.blocks_count-1) / _sb.blocks_per_group) + 1;
}

void FS::_writeSuperBlock() {
    _part->writeaddr(1024, &_sb, sizeof(SuperBlock));

    //to write the super block backup with the sparse superblock feature:
    //for(uint i = 1; i < _nbBgd; i *= {3,5,7}) {
        //SuperBlock sb;
        //_part->readaddr((_sb.first_data_block + i*_sb.blocks_per_group)*_blockSize, &sb, sizeof(SuperBlock));
        //printf("%d %x\n", _sb.first_data_block + i*_sb.blocks_per_group, sb.magic);
    //}
}

void FS::_loadBlockGroupDescriptor() {
    if(_bgd != nullptr) {
        free(_bgd);
    }

    _bgd = (BlockGroupDescriptor*)malloc(_nbBgd*sizeof(BlockGroupDescriptor));
    uint addr = (_sb.first_data_block+1)*_blockSize;
    _part->readaddr(addr, _bgd, _nbBgd*sizeof(BlockGroupDescriptor));
}

void FS::_writeBlockGroupDescriptor() {
    uint addr = (_sb.first_data_block+1)*_blockSize;
    _part->writeaddr(addr, _bgd, _nbBgd*sizeof(BlockGroupDescriptor));
}

File::File(u32 inode, InodeData data, FS* fs) :
    _inode(inode), _data(data), _fs(fs) {}

void File::readaddr(u64 addr, void* data, size_t size) const {
    ReadRecArgs args;
    args.addr = addr;
    args.data = (u8*)data;
    args.size = size;
    args.indirectSize[0] = _fs->_blockSize;
    for(int i = 0; i < 3; ++i) {
        args.indirectSize[i+1] = (_fs->_blockSize/4)*args.indirectSize[i];
        args.blocks[i] = nullptr;
    }

    for(int j = 0; j < 12; ++j) {
        _readrec(args, 0, _data.block[j]);
    }
    for(int i = 0; i < 3; ++i) {
        args.blocks[i] = (u32*)malloc(_fs->_blockSize);
        _readrec(args, i+1, _data.block[12+i]);
    }

    for(int i = 0; i < 3; ++i) {
        if(args.blocks[i] != nullptr) {
            free(args.blocks[i]);
        }
    }
}

void File::_readrec(ReadRecArgs& args, int level, u32 blockId) const {
    if(args.size == 0) return;
    if(args.addr >= args.indirectSize[level]) {
        args.addr -= args.indirectSize[level];
        return;
    }

    if(level == 0) {
        uint count = min(_fs->_blockSize - args.addr, (u64)args.size);
        _fs->_part->readaddr(blockId*_fs->_blockSize + args.addr, args.data, count);
        args.data += count;
        args.size -= count;
        args.addr = 0;
    } else {
        _fs->_part->readaddr(blockId*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);

        uint starti = args.addr/args.indirectSize[level-1];
        args.addr %= args.indirectSize[level-1];

        for(uint i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.size == 0) return;
            _readrec(args, level-1, args.blocks[level-1][i]);
        }
    }
}

void File::writeaddr(u64 addr, const void* data, size_t size) {
    WriteRecArgs args;
    args.addr = addr;
    args.data = (u8*)data;
    args.size = size;
    args.blockNum = 0;
    args.indirectSize[0] = _fs->_blockSize;
    args.indirectBlock[0] = 1;
    for(int i = 0; i < 3; ++i) {
        args.indirectSize[i+1] = (_fs->_blockSize/4)*args.indirectSize[i];
        args.indirectBlock[i+1] = 1+(_fs->_blockSize/4)*args.indirectBlock[i];
        args.blocks[i] = nullptr;
    }

    bool write = false;
    for(int j = 0; j < 12; ++j) {
        write |= _writerec(args, 0, &_data.block[j]);
    }
    for(int i = 0; i < 3; ++i) {
        args.blocks[i] = (u32*)malloc(_fs->_blockSize);
        write |= _writerec(args, i+1, &_data.block[12+i]);
    }

    for(int i = 0; i < 3; ++i) {
        if(args.blocks[i] != nullptr) {
            free(args.blocks[i]);
        }
    }

    size_t newSize = addr + size;
    if(newSize > _data.getSize()) {
        _data.setSize(newSize);
        write = true;
    }
    if(write) {
        _fs->writeInodeData(_inode, &_data);
    }
}

bool File::_writerec(WriteRecArgs& args, int level, u32* blockId) {
    if(args.size == 0) return false;

    bool canSkip = args.blockNum + args.indirectBlock[level] - 1 < _data.getBlockCount(_fs->_sb);

    if(canSkip && (args.addr >= args.indirectSize[level])) {
        args.blockNum += args.indirectBlock[level];
        args.addr -= args.indirectSize[level];
        return false;
    }

    if(level == 0) {
        bool retVal = false;
        if(args.blockNum >= _data.getBlockCount(_fs->_sb)) {
            *blockId = _fs->getNewBlock(_inode);
            _data.incrBlockCount(1, _fs->_sb);
            retVal = true;
        }

        if(args.addr < _fs->_blockSize) {
            uint count = min(_fs->_blockSize - args.addr, (u64)args.size);
            _fs->_part->writeaddr((*blockId)*_fs->_blockSize + args.addr, args.data, count);
            args.data += count;
            args.size -= count;
            args.addr = 0;
        } else {
            args.addr -= _fs->_blockSize;
        }

        args.blockNum += 1;
        return retVal;
    } else {
        bool retVal = false;

        if(args.blockNum >= _data.getBlockCount(_fs->_sb)) {
            assert(args.blockNum == _data.getBlockCount(_fs->_sb));
            *blockId = _fs->getNewBlock(_inode);
            _data.incrBlockCount(1, _fs->_sb);
            retVal = true;
            memset(args.blocks[level-1], 0, _fs->_blockSize);
        } else {
            _fs->_part->readaddr((*blockId)*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);
        }

        args.blockNum += 1;

        uint starti = 0;
        if(canSkip) {
            starti = args.addr/args.indirectSize[level-1];
            args.addr %= args.indirectSize[level-1];
            args.blockNum += starti*args.indirectBlock[level-1];
        }

        bool write = false;
        for(uint i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.size == 0) {
                break;
            }
            write |= _writerec(args, level-1, &args.blocks[level-1][i]);
        }
        assert(!retVal || write); //retVal => write
        if(write) {
            _fs->_part->writeaddr((*blockId)*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);
        }
        return retVal;
    }
}

void File::resize(size_t size) {
    if(size < _data.getSize()) {
        ResizeRecArgs args;
        args.sizeAfter = size;
        args.sizeBefore = alignup(_data.size, _fs->_blockSize);
        args.indirectSize[0] = _fs->_blockSize;
        for(int i = 0; i < 3; ++i) {
            args.indirectSize[i+1] = (_fs->_blockSize/4)*args.indirectSize[i];
            args.blocks[i] = nullptr;
        }

        for(int j = 0; j < 12; ++j) {
            _resizerec(args, 0, _data.block[j]);
        }
        for(int i = 0; i < 3; ++i) {
            args.blocks[i] = (u32*)malloc(_fs->_blockSize);
            _resizerec(args, i+1, _data.block[12+i]);
        }

        for(int i = 0; i < 3; ++i) {
            if(args.blocks[i] != nullptr) {
                free(args.blocks[i]);
            }
        }

        _data.setSize(size);
        _fs->writeInodeData(_inode, &_data);
    } else {
        writeaddr(size, nullptr, 0);
    }
}

bool File::_resizerec(ResizeRecArgs& args, int level, u32 blockId) {
    if(args.sizeBefore == 0) return false;
    if(args.sizeAfter >= args.indirectSize[level]) {
        args.sizeAfter  -= args.indirectSize[level];
        args.sizeBefore -= args.indirectSize[level];
        return false;
    }

    if(level == 0) {
        if(args.sizeAfter != 0) {
            args.sizeAfter = 0;
            args.sizeBefore -= _fs->_blockSize;
            return false;
        }
        _fs->freeBlock(blockId);
        _data.incrBlockCount(-1, _fs->_sb);
        args.sizeBefore -= _fs->_blockSize;
        return true;
    } else {
        _fs->_part->readaddr(blockId*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);

        uint starti = args.sizeAfter/args.indirectSize[level-1];
        args.sizeAfter  -= starti*args.indirectSize[level-1];
        args.sizeBefore -= starti*args.indirectSize[level-1];

        bool deleteMyself = (starti == 0);
        for(uint i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.sizeBefore == 0) break;
            if(_resizerec(args, level-1, args.blocks[level-1][i])) {
                args.blocks[level-1][i] = 0;
            } else {
                deleteMyself = false;
            }
        }

        _fs->_part->writeaddr(blockId*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);

        if(deleteMyself) {
            _fs->freeBlock(blockId);
            _data.incrBlockCount(-1, _fs->_sb);
            return true;
        }
        return false;
    }
}

void File::link() {
    _data.links_count += 1;
    _fs->writeInodeData(_inode, &_data);
}

void File::unlink() {
    if(_data.links_count == 1) {
        resize(0);
        _fs->freeInode(_inode, S_ISDIR(_data.mode));
        memset(&_data, 0, sizeof(InodeData));
    } else {
        _data.links_count -= 1;
    }
    _fs->writeInodeData(_inode, &_data);
}

void File::getStats(stat* buf) {
    buf->st_ino = _inode;
    buf->st_mode = _data.mode;
    buf->st_nlink = _data.links_count;
    buf->st_uid = _data.uid;
    buf->st_gid = _data.gid;
    buf->st_size = _data.getSize();
    buf->st_blocks = _data.blocks;
    buf->st_atim.tv_nsec = 0;
    buf->st_mtim.tv_nsec = 0;
    buf->st_ctim.tv_nsec = 0;
    buf->st_atim.tv_sec = _data.atime;
    buf->st_mtim.tv_sec = _data.mtime;
    buf->st_ctim.tv_sec = _data.ctime;
}

size_t File::getSize() const {
    return _data.getSize();
}

Directory::Directory(u32 inode, InodeData data, FS* fs) : File(inode, data, fs) { }

File* Directory::operator[](const std::string& name) {
    u32 inode = 0;
    void* d = open();
    dirent* dir;
    while((dir = read(d)) != nullptr) {
        if(std::string(dir->d_name) == name) {
            inode = dir->d_ino;
        }
    }
    close(d);

    if(inode == 0) {
        return nullptr;
    }
    InodeData inodedat;
    _fs->getInodeData(inode, &inodedat);
    if(S_ISDIR(inodedat.mode)) {
        return new Directory(inode, inodedat, _fs);
    } else if(S_ISREG(inodedat.mode)) {
        return new File(inode, inodedat, _fs);
    } else {
        return nullptr;
    }
}

void* Directory::open() {
    DirIterator* res = (DirIterator*)malloc(sizeof(DirIterator));
    res->pos = 0;
    return res;
}

dirent* Directory::read(void* d) {
    DirIterator* dirit = (DirIterator*)d;

    do {
        if(dirit->pos >= _data.blocks*512) return nullptr;
        readaddr(dirit->pos, &dirit->entry, sizeof(DirectoryEntry));
        readaddr(dirit->pos+sizeof(DirectoryEntry), &dirit->res.d_name, dirit->entry.name_len);
        dirit->pos += dirit->entry.rec_len;
    } while(dirit->entry.inode == 0);

    dirit->res.d_name[dirit->entry.name_len] = 0;
    dirit->res.d_ino = dirit->entry.inode;
    return &dirit->res;
}

long int Directory::tell(void* d) {
    return (long int)(((DirIterator*)d)->pos);
}

void Directory::seek(void* d, long int loc) {
    ((DirIterator*)d)->pos = loc;
}

void Directory::close(void* d) {
    free(d);
}

static DirectoryFileType inodeToDirType(u16 mode) {
    if(S_ISREG(mode))  return FT_REG_FILE;
    if(S_ISDIR(mode))  return FT_DIR;
    if(S_ISCHR(mode))  return FT_CHRDEV;
    if(S_ISBLK(mode))  return FT_BLKDEV;
    if(S_ISFIFO(mode)) return FT_FIFO;
    if(S_ISSOCK(mode)) return FT_SOCK;
    if(S_ISLNK(mode))  return FT_SYMLINK;
    return FT_UNKNOWN;
}

void Directory::addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) {
    ::File* f = nullptr;
    if(S_ISREG(mode)) {
        f = _fs->getNewFile(uid, gid, mode);
    } else if(S_ISDIR(mode)) {
        f = _fs->getNewDirectory(uid, gid, mode);
    } else {
        bsod("Ext2::Directory::addEntry: me dunno wat to do wiz mode %u\n", mode);
    }
    addEntry(name, f);
    f->link();
}

void Directory::addEntry(const std::string& name, ::File* file) {
    if(file->getType() ==  FileType::Directory && std::string("..") != name) {
        file->dir()->addEntry("..", this);
        link();
    }
    u32 neededSize = sizeof(DirectoryEntry) + name.size();
    stat stats;
    file->getStats(&stats);
    u32 inode = stats.st_ino;
    DirectoryEntry entry;
    u32 curPos = 0;
    u32 nextPos = 0;

    while(true) {
        curPos = nextPos;
        if(curPos >= _data.blocks*512) {
            bsod("Ext2::Directory::addFile: It should be impossible to get here\n");
        }
        readaddr(curPos, &entry, sizeof(DirectoryEntry));
        nextPos = curPos + entry.rec_len;

        u32 curSize = alignup((u32)(sizeof(DirectoryEntry) + entry.name_len), (u32)4);
        u32 availableSize = entry.rec_len - curSize;

        bool atEnd = nextPos >= _data.blocks*512;

        if(atEnd || neededSize < availableSize) {
            u32 targetPos = 0;
            //a directory record must not be on two blocks
            if((curPos+curSize)/_fs->_blockSize == (curPos + curSize + neededSize)/_fs->_blockSize) {
                targetPos = curPos + curSize;
            } else {
                targetPos = alignup(curPos+curSize, _fs->_blockSize);
                if(!(atEnd || targetPos + neededSize < curPos + entry.rec_len)) {
                    continue;
                }
            }
            DirectoryEntry newEntry;
            newEntry.inode = inode;
            if(atEnd) {
                newEntry.rec_len = _data.blocks*512 - targetPos;
            } else {
                newEntry.rec_len = nextPos - targetPos;
            }
            newEntry.name_len = name.size();
            InodeData dat;
            _fs->getInodeData(inode, &dat);
            newEntry.type = inodeToDirType(dat.mode);
            writeaddr(targetPos, &newEntry, sizeof(DirectoryEntry));
            writeaddr(targetPos+sizeof(DirectoryEntry), name.c_str(), name.size());
            entry.rec_len = targetPos - curPos;
            writeaddr(curPos, &entry, sizeof(DirectoryEntry));
            break;
        }
    }
}


void Directory::removeFile(const std::string& name) {
    File* f = (*this)[name];
    assert(f != nullptr);
    f->unlink();
    removeEntry(name);
}

void Directory::removeDirectory(const std::string& name) {
    File* f = (*this)[name];
    assert(f != nullptr);
    assert(f->getType() == FileType::Directory);
    removeEntry(name);
    f->dir()->deleteDir();
}


void Directory::removeEntry(const std::string& name) {
    DirIterator* d = (DirIterator*)open();
    u32 lastPos = 0;
    u32 curPos = 0;
    u32 inode;
    u32 rec_len;
    u8 type = FT_UNKNOWN;
    dirent* dir;
    while(true) {
        lastPos = curPos;
        curPos = d->pos;
        dir = read(d);
        if(dir == nullptr) break;
        inode = d->entry.inode;
        type = d->entry.type;
        rec_len = d->entry.rec_len;
        if(std::string(dir->d_name) == name) {
            break;
        }
    }
    close(d);
    if(dir == nullptr) {
        bsod("Ext2::Directory::removeEntry called with a file that doesn't exists!\n");
    }

    if(type == FT_DIR && std::string("..") != name) {
        InodeData dat;
        _fs->getInodeData(inode, &dat);
        Directory directory(inode, dat, _fs);
        directory.removeEntry("..");
        unlink();
    }

    if(curPos == 0) {
        DirectoryEntry entry;
        entry.inode = 0;
        entry.name_len = 0;
        entry.type = FT_UNKNOWN;
        writeaddr(0, &entry, sizeof(DirectoryEntry));
        return;
    }

    DirectoryEntry lastEntry;
    readaddr(lastPos, &lastEntry, sizeof(DirectoryEntry));
    lastEntry.rec_len += rec_len;
    writeaddr(lastPos, &lastEntry, sizeof(DirectoryEntry));
}

void Directory::deleteDir() {
    void* d = open();
    dirent* dir;
    while((dir = read(d)) != nullptr) {
        if(dir->d_ino == _inode) {
            unlink();
        } else {
            InodeData dat;
            _fs->getInodeData(dir->d_ino, &dat);
            File f(dir->d_ino, dat, _fs);
            f.unlink();
        }
    }
    close(d);
    unlink();
}

void Directory::init() {
    DirectoryEntry entry;
    entry.inode = _inode;
    entry.rec_len = _fs->_blockSize;
    entry.name_len = 1;
    entry.type = FT_DIR;
    char name = '.';
    resize(_fs->_blockSize);
    writeaddr(0, &entry, sizeof(DirectoryEntry));
    writeaddr(sizeof(DirectoryEntry), &name, 1);
    link(); //reference for '.'
}

}

