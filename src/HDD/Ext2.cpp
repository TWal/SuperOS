#include "Ext2.h"

namespace HDD {

namespace Ext2 {

/*  ___                 _      ____        _
   |_ _|_ __   ___   __| | ___|  _ \  __ _| |_ __ _ 
    | || '_ \ / _ \ / _` |/ _ \ | | |/ _` | __/ _` |
    | || | | | (_) | (_| |  __/ |_| | (_| | || (_| |
   |___|_| |_|\___/ \__,_|\___|____/ \__,_|\__\__,_|
*/

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

/*  _____ ____  
   |  ___/ ___|
   | |_  \___ \
   |  _|  ___) |
   |_|   |____/
*/

FS::FS(Partition* part) : _part(part) {
    _bgd = nullptr;
    _loadSuperBlock();
    assert(_sb.magic == 0xef53);
    assert(_sb.log_block_size == (u32)_sb.log_frag_size);
    _loadBlockGroupDescriptor();
}

std::unique_ptr<::HDD::Directory> FS::getRoot() {
    InodeData dat;
    getInodeData(2, &dat);
    return std::unique_ptr<::HDD::Directory>(new Directory(this, 2, dat));
}

RegularFile* FS::getNewFile(u16 uid, u16 gid, u16 mode) {
    assert(S_ISREG(mode)); //TODO handle this better
    u32 inode = getNewInode(false);
    InodeData dat;
    memset(&dat, 0, sizeof(InodeData));
    dat.uid = uid;
    dat.gid = gid;
    dat.mode = mode;
    writeInodeData(inode, &dat);
    return new RegularFile(this, inode, dat);
}

Directory* FS::getNewDirectory(u16 uid, u16 gid, u16 mode) {
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
    Directory* d = new Directory(this, inode, dat);
    d->init();
    return d;
}


void FS::getInodeData(u32 inode, InodeData* res) const {
    u64 blockGroup = (inode-1) / _sb.inodes_per_group;
    u64 index      = (inode-1) % _sb.inodes_per_group;
    u64 inodeTable = _bgd[blockGroup].inode_table;
    _part->readaddr(inodeTable*_blockSize + index*_sb.inode_size, res, sizeof(InodeData));
}

void FS::writeInodeData(u32 inode, const InodeData* data) {
     u64 blockGroup = (inode-1) / _sb.inodes_per_group;
     u64 index      = (inode-1) % _sb.inodes_per_group;
     u64 inodeTable = _bgd[blockGroup].inode_table;
    _part->writeaddr(inodeTable*_blockSize + index*_sb.inode_size, data, sizeof(InodeData));
}

u32 FS::getNewBlock(u32 nearInode) {
    //TODO: some bitmap caching to speed up writing
    u64 blockGroup = (nearInode-1) / _sb.inodes_per_group;
    u8* bitmap = (u8*)malloc(_blockSize);
    for(u64 i = 0; i < _nbBgd; ++i) {
        u64 bgd = (i+blockGroup)%_nbBgd;
        if(_bgd[bgd].free_blocks_count == 0) continue;
        _part->readaddr(_bgd[bgd].block_bitmap*_blockSize, bitmap, _blockSize);
        for(u64 j = 0; j < _blockSize; ++j) {
            if((u8)~bitmap[j] != 0) {
                u64 pos = __builtin_ctz(~bitmap[j]);
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
    for(u64 i = 0; i < _nbBgd; ++i) {
        if(_bgd[i].free_inodes_count == 0) continue;
        _part->readaddr(_bgd[i].inode_bitmap*_blockSize, bitmap, _blockSize);
        for(u64 j = 0; j < _blockSize; ++j) {
            if((u8)~bitmap[j] != 0) {
                u64 pos = __builtin_ctz(~bitmap[j]);
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
    //for(u64 i = 1; i < _nbBgd; i *= {3,5,7}) {
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
    u64 addr = (_sb.first_data_block+1)*_blockSize;
    _part->readaddr(addr, _bgd, _nbBgd*sizeof(BlockGroupDescriptor));
}

void FS::_writeBlockGroupDescriptor() {
    u64 addr = (_sb.first_data_block+1)*_blockSize;
    _part->writeaddr(addr, _bgd, _nbBgd*sizeof(BlockGroupDescriptor));
}


/*  ___                 _
   |_ _|_ __   ___   __| | ___
    | || '_ \ / _ \ / _` |/ _ \
    | || | | | (_) | (_| |  __/
   |___|_| |_|\___/ \__,_|\___|
*/

Inode::Inode(FS* fs, u32 inode, InodeData data) :
    _fs(fs), _inode(inode), _data(data) {}

void Inode::i_readaddr(u64 addr, void* data, size_t size) const {
    readData();
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
        readrec(args, 0, _data.block[j]);
    }
    for(int i = 0; i < 3; ++i) {
        args.blocks[i] = (u32*)malloc(_fs->_blockSize);
        readrec(args, i+1, _data.block[12+i]);
    }

    for(int i = 0; i < 3; ++i) {
        if(args.blocks[i] != nullptr) {
            free(args.blocks[i]);
        }
    }
}

void Inode::readrec(ReadRecArgs& args, int level, u32 blockId) const {
    if(args.size == 0) return;
    if(args.addr >= args.indirectSize[level]) {
        args.addr -= args.indirectSize[level];
        return;
    }

    if(level == 0) {
        u64 count = min(_fs->_blockSize - args.addr, (u64)args.size);
        _fs->_part->readaddr(blockId*_fs->_blockSize + args.addr, args.data, count);
        args.data += count;
        args.size -= count;
        args.addr = 0;
    } else {
        _fs->_part->readaddr(blockId*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);

        u64 starti = args.addr/args.indirectSize[level-1];
        args.addr %= args.indirectSize[level-1];

        for(u64 i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.size == 0) return;
            readrec(args, level-1, args.blocks[level-1][i]);
        }
    }
}

void Inode::i_writeaddr(u64 addr, const void* data, size_t size) {
    readData();
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
        write |= writerec(args, 0, &_data.block[j]);
    }
    for(int i = 0; i < 3; ++i) {
        args.blocks[i] = (u32*)malloc(_fs->_blockSize);
        write |= writerec(args, i+1, &_data.block[12+i]);
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

bool Inode::writerec(WriteRecArgs& args, int level, u32* blockId) {
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
            u64 count = min(_fs->_blockSize - args.addr, (u64)args.size);
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

        u64 starti = 0;
        if(canSkip) {
            starti = args.addr/args.indirectSize[level-1];
            args.addr %= args.indirectSize[level-1];
            args.blockNum += starti*args.indirectBlock[level-1];
        }

        bool write = false;
        for(u64 i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.size == 0) {
                break;
            }
            write |= writerec(args, level-1, &args.blocks[level-1][i]);
        }
        assert(!retVal || write); //retVal => write
        if(write) {
            _fs->_part->writeaddr((*blockId)*_fs->_blockSize, args.blocks[level-1], _fs->_blockSize);
        }
        return retVal;
    }
}

void Inode::i_resize(size_t size) {
    readData();
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
            resizerec(args, 0, _data.block[j]);
        }
        for(int i = 0; i < 3; ++i) {
            args.blocks[i] = (u32*)malloc(_fs->_blockSize);
            resizerec(args, i+1, _data.block[12+i]);
        }

        for(int i = 0; i < 3; ++i) {
            if(args.blocks[i] != nullptr) {
                free(args.blocks[i]);
            }
        }

        _data.setSize(size);
        _fs->writeInodeData(_inode, &_data);
    } else {
        i_writeaddr(size, nullptr, 0);
    }
}

bool Inode::resizerec(ResizeRecArgs& args, int level, u32 blockId) {
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

        u64 starti = args.sizeAfter/args.indirectSize[level-1];
        args.sizeAfter  -= starti*args.indirectSize[level-1];
        args.sizeBefore -= starti*args.indirectSize[level-1];

        bool deleteMyself = (starti == 0);
        for(u64 i = starti; i < _fs->_blockSize/4; ++i) {
            if(args.sizeBefore == 0) break;
            if(resizerec(args, level-1, args.blocks[level-1][i])) {
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

void Inode::link() {
    readData();
    _data.links_count += 1;
    writeData();
}

void Inode::unlink() {
    readData();
    if(_data.links_count == 1) {
        i_resize(0);
        _fs->freeInode(_inode, S_ISDIR(_data.mode));
        memset(&_data, 0, sizeof(InodeData));
    } else {
        _data.links_count -= 1;
    }
    writeData();
}

void Inode::i_getStats(stat* buf) const {
    readData();
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

size_t Inode::i_getSize() const {
    readData();
    return _data.getSize();
}

void Inode::writeData() {
    _fs->writeInodeData(_inode, &_data);
}

void Inode::readData() const {
    //Here, "const" means "won't write on the hard drive"
    _fs->getInodeData(_inode, (InodeData*)&_data);
}



/*  ____                  _            _____ _ _
   |  _ \ ___  __ _ _   _| | __ _ _ __|  ___(_) | ___
   | |_) / _ \/ _` | | | | |/ _` | '__| |_  | | |/ _ \
   |  _ <  __/ (_| | |_| | | (_| | |  |  _| | | |  __/
   |_| \_\___|\__, |\__,_|_|\__,_|_|  |_|   |_|_|\___|
              |___/
*/

RegularFile::RegularFile(FS* fs, u32 inode, InodeData data) :
    Inode(fs, inode, data) {}

void RegularFile::readaddr(u64 addr, void* data, size_t size) const {
    i_readaddr(addr, data, size);
}

void RegularFile::writeaddr(u64 addr, const void* data, size_t size) {
    i_writeaddr(addr, data, size);
}

void RegularFile::resize(size_t size) {
    i_resize(size);
}

void RegularFile::getStats(stat* buf) const {
    i_getStats(buf);
}

size_t RegularFile::getSize() const {
    return i_getSize();
}

/*  ____  _               _
   |  _ \(_)_ __ ___  ___| |_ ___  _ __ _   _ 
   | | | | | '__/ _ \/ __| __/ _ \| '__| | | |
   | |_| | | | |  __/ (__| || (_) | |  | |_| |
   |____/|_|_|  \___|\___|\__\___/|_|   \__, |
                                        |___/
*/

Directory::Directory(FS* fs, u32 inode, InodeData data) :
    Inode(fs, inode, data) {}

void Directory::getStats(stat* buf) const {
    i_getStats(buf);
}

std::unique_ptr<::HDD::File> Directory::operator[](const std::string& name) {
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
        return std::unique_ptr<::HDD::File>(new Directory(_fs, inode, inodedat));
    } else if(S_ISREG(inodedat.mode)) {
        return std::unique_ptr<::HDD::File>(new RegularFile(_fs, inode, inodedat));
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
        i_readaddr(dirit->pos, &dirit->entry, sizeof(DirectoryEntry));
        i_readaddr(dirit->pos+sizeof(DirectoryEntry), &dirit->res.d_name, dirit->entry.name_len);
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

std::unique_ptr<::HDD::File> Directory::addEntry(const std::string& name, u16 uid, u16 gid, u16 mode) {
    if(S_ISREG(mode)) {
        RegularFile* f = _fs->getNewFile(uid, gid, mode);
        addEntry(name, f);
        return std::unique_ptr<::HDD::File>(f);
    } else if(S_ISDIR(mode)) {
        Directory* f = _fs->getNewDirectory(uid, gid, mode);
        addEntry(name, f);
        return std::unique_ptr<::HDD::File>(f);
    } else {
        bsod("HDD::Ext2::Directory::addEntry: me dunno wat to do wiz mode %u\n", mode);
    }
}

void Directory::addEntry(const std::string& name, ::HDD::File* file) {
    if(file->getType() ==  FileType::Directory && std::string("..") != name) {
        static_cast<::HDD::Directory*>(file)->addEntry("..", this);
    }

    //link
    switch(file->getType()) {
        case FileType::RegularFile:
            static_cast<RegularFile*>(file)->link();
            break;
        case FileType::Directory:
            static_cast<Directory*>(file)->link();
            break;
        default:
            bsod("HDD::Ext2::Directory::adEntry: me dunno filetype %u\n", file->getType());
            break;
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
            bsod("HDD::Ext2::Directory::addFile: It should be impossible to get here\n");
        }
        i_readaddr(curPos, &entry, sizeof(DirectoryEntry));
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
                if(!(atEnd || targetPos + neededSize < nextPos)) {
                    continue;
                }
            }
            DirectoryEntry newEntry;
            newEntry.inode = inode;
            if(atEnd) {
                newEntry.rec_len = alignup(targetPos+neededSize, _fs->_blockSize) - targetPos;
            } else {
                newEntry.rec_len = nextPos - targetPos;
            }
            newEntry.name_len = name.size();
            InodeData dat;
            _fs->getInodeData(inode, &dat);
            newEntry.type = inodeToDirType(dat.mode);
            i_writeaddr(targetPos, &newEntry, sizeof(DirectoryEntry));
            if(targetPos + neededSize < newEntry.rec_len) {
                i_writeaddr(targetPos+sizeof(DirectoryEntry), name.c_str(), name.size()+1);
            } else {
                i_writeaddr(targetPos+sizeof(DirectoryEntry), name.c_str(), name.size());
            }
            entry.rec_len = targetPos - curPos;
            i_writeaddr(curPos, &entry, sizeof(DirectoryEntry));
            break;
        }
    }
}

void Directory::removeDirectory(const std::string& name) {
    std::unique_ptr<::HDD::File> f = (*this)[name];
    assert(f);
    assert(f->getType() == FileType::Directory);
    removeEntry(name);
    static_cast<Directory*>(f.get())->deleteDir();
}


void Directory::removeEntry(const std::string& name) {
    DirIterator* d = (DirIterator*)open();
    u32 lastPos = 0;
    u32 curPos = 0;
    u32 inode = 0;
    u32 rec_len = 0;
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
        bsod("HDD::Ext2::Directory::removeEntry called with a file that doesn't exists!\n");
    }

    InodeData dat;
    _fs->getInodeData(inode, &dat);
    Inode(_fs, inode, dat).unlink();;
    if(type == FT_DIR && std::string("..") != name) {
        Directory directory(_fs, inode, dat);
        directory.removeEntry("..");
    }

    if(curPos == 0) {
        DirectoryEntry entry;
        entry.inode = 0;
        entry.name_len = 0;
        entry.type = FT_UNKNOWN;
        i_writeaddr(0, &entry, sizeof(DirectoryEntry));
        return;
    }

    DirectoryEntry lastEntry;
    i_readaddr(lastPos, &lastEntry, sizeof(DirectoryEntry));
    lastEntry.rec_len += rec_len;
    i_writeaddr(lastPos, &lastEntry, sizeof(DirectoryEntry));
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
            RegularFile f(_fs, dir->d_ino, dat);
            f.unlink();
        }
    }
    close(d);
}

void Directory::init() {
    DirectoryEntry entry;
    entry.inode = _inode;
    entry.rec_len = _fs->_blockSize;
    entry.name_len = 1;
    entry.type = FT_DIR;
    const char* name = ".\0";
    i_resize(_fs->_blockSize);
    i_writeaddr(0, &entry, sizeof(DirectoryEntry));
    i_writeaddr(sizeof(DirectoryEntry), name, 2);
    link(); //reference for '.'
}

} //end of namespace Ext2

} //end of namespace HDD

