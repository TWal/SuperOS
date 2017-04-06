#include "Ext2.h"
#include "../IO/FrameBuffer.h"

namespace Ext2 {

u32 InodeData::getBlockCount(const SuperBlock& sb) const {
    return blocks/(2<<sb.log_block_size);
}

void InodeData::incrBlockCount(int diff, const SuperBlock& sb) {
    blocks += diff*(2<<sb.log_block_size);
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
    uint nbBgd = ((_sb.blocks_count-1) / _sb.blocks_per_group) + 1;
    u8* bitmap = (u8*)malloc(_blockSize);
    for(uint i = 0; i < nbBgd; ++i) {
        uint bgd = (i+blockGroup)%nbBgd;
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
    if(newSize > _data.size) {
        _data.size = newSize;
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
    if(size < _data.size) {
        ResizeRecArgs args;
        args.sizeAfter = size;
        args.sizeBefore = ((_data.size - 1) / _fs->_blockSize + 1) * _fs->_blockSize; //align on upper blockSize multiple
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

        _data.size = size;
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


size_t File::getSize() const {
    return _data.size;
}


Directory::Directory(u32 inode, InodeData data, FS* fs) : File(inode, data, fs) { }

std::vector<std::string> Directory::getFilesName() {
    std::vector<std::string> res;
    for(dirent d : *this) {
        res.push_back(d.d_name);
    }
    return res;
}

File* Directory::operator[](const std::string& name) {
    uint inode = 0;
    for(dirent d : *this) {
        if(std::string(d.d_name) == name) {
            inode = d.d_ino;
        }
    }
    if(inode == 0) {
        return nullptr;
    }
    InodeData inodedat;
    _fs->getInodeData(inode, &inodedat);
    if((inodedat.mode&0xE000) == FM_IFDIR) {
        return new Directory(inode, inodedat, _fs);
    } else if((inodedat.mode&0xE000) == FM_IFREG) {
        return new File(inode, inodedat, _fs);
    } else {
        return nullptr;
    }
}

Directory::iterator Directory::begin() {
    return iterator(this, 0);
}

Directory::iterator Directory::end() {
    return iterator(this);
}

Directory::iterator& Directory::iterator::operator++() {
    new(this) iterator(_father, _pos + _entry.rec_len);
    return *this;
}

dirent Directory::iterator::operator*() {
    dirent res;
    res.d_ino = _entry.inode;
    strncpy(res.d_name, _name, 256);
    return res;
}

Directory::iterator::iterator(Directory* father, u32 pos) {
    _father = father;
    _pos = pos;
    _father->readaddr(_pos, &_entry, sizeof(DirectoryEntry));
    _father->readaddr(_pos+sizeof(DirectoryEntry), &_name, _entry.name_len);
    _name[_entry.name_len] = 0;
}

Directory::iterator::iterator(Directory* father) {
    _father = father;
    _entry.inode = 0;
}

bool Directory::iterator::operator==(const iterator& other) {
    return _entry.inode == other._entry.inode && _father == other._father;
}

bool Directory::iterator::operator!=(const iterator& other) {
    return !((*this) == other);
}

}

