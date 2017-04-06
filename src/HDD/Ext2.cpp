#include "Ext2.h"
#include "../IO/FrameBuffer.h"

namespace Ext2 {

FS::FS(Partition* part) : FileSystem(part) {
    _bgd = nullptr;
    _loadSuperBlock();
    assert(_sb.magic == 0xef53);
    assert(_sb.log_block_size == (u32)_sb.log_frag_size);
    loadBlockGroupDescriptor();

    //uint nbBgd = ((_sb.blocks_count-1) / _sb.blocks_per_group) + 1;

    //for(uint i = 1; i < nbBgd; i *= {3,5,7}) {
        //SuperBlock sb;
        //_part->readaddr((_sb.first_data_block + i*_sb.blocks_per_group)*_blockSize, &sb, sizeof(SuperBlock));
        //printf("%d %x\n", _sb.first_data_block + i*_sb.blocks_per_group, sb.magic);
    //}
}

u32 FS::getNewBlock(u32 nearInode) {
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
                free(bitmap);
                return _sb.blocks_per_group*bgd + 8*sizeof(u8)*j + pos;
            }
        }
    }
    free(bitmap);
    bsod("No space on disk :-("); //TODO handle this properly
    return 42; //make g++ happy
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

void FS::_loadSuperBlock() {
    _part->readlba(2, &_sb, 2);
    if(_sb.rev_level == 0) {
        _sb.inode_size = 128;
    }
    _blockSize = 1024 << _sb.log_block_size;
}

void FS::_writeSuperBlock() {
}

void FS::loadBlockGroupDescriptor() {
    if(_bgd != nullptr) {
        free(_bgd);
    }

    uint nbBgd = ((_sb.blocks_count-1) / _sb.blocks_per_group) + 1;
    uint bgdSize = nbBgd * sizeof(BlockGroupDescriptor);
    _bgd = (BlockGroupDescriptor*)malloc(bgdSize);

    uint addr = (_sb.first_data_block+1)*_blockSize;

    _part->readaddr(addr, _bgd, bgdSize);
}

File::File(u32 inode, InodeData data, FS* fs) :
    _inode(inode), _data(data), _fs(fs) {}

static u64 getBlockId(int i, uint* is, u64* indirectSize) {
    if(i < 12) {
        return i;
    }
    i -= 12;
    u64 blockId = 12;
    for(int j = 1; j <= i; ++j) {
        blockId += indirectSize[j];
    }
    for(int j = i; j >= 0; --j) {
        blockId += is[j]*indirectSize[i-j];
    }
    return blockId;
}

void File::writeaddr(u64 addr, const void* data, size_t size) {
    void* pdata = &data;

    _staraddr(pdata, addr, size,
        //gtBlock
        [](File* self, u32* block, uint j, uint* is, u64* indirectSize, uint i, void* buffer) {
            FS* fs = self->_fs;
            if(getBlockId(i, is, indirectSize) >= (self->_data.blocks/(2<<fs->_sb.log_block_size))) {
                block[j] = fs->getNewBlock(self->_inode);
                return true; //since this is a new block, there is garbage inside so we don't care
            }
            fs->_part->readaddr(block[j]*fs->_blockSize, buffer, fs->_blockSize);
            return false;
        },
        //doWork
        [](File* self, void* pdata, size_t blockId, size_t addr, size_t count) {
            FS* fs = self->_fs;
            u8*& data = *(u8**)pdata;
            fs->_part->writeaddr(blockId*fs->_blockSize + addr, data, count);
            data += count;
        },
        //skip
        [](File* self, int i, uint* is, u64* indirectSize) {
            FS* fs = self->_fs;
            return getBlockId(i, is, indirectSize) + indirectSize[i-12+1] < (self->_data.blocks/(2<<fs->_sb.log_block_size));
        },
        //prepareData
        [](File* self, u32* block, size_t j, uint* is, u64* indirectSize, uint i) {
            FS* fs = self->_fs;
            if(getBlockId(i, is, indirectSize) >= (self->_data.blocks/(2<<fs->_sb.log_block_size))) {
                block[j] = fs->getNewBlock(self->_inode);
                char* s = (char*)malloc(fs->_blockSize);
                memset(s, 0, fs->_blockSize);
                fs->_part->writeaddr(block[j]*fs->_blockSize, s, fs->_blockSize);
                free(s);
                self->_data.blocks += (2<<fs->_sb.log_block_size);
                return true;
            }
            return false;
        }
    );
}


void File::readaddr(u64 addr, void* data, size_t size) const {
    void* pdata = &data;
    ((File*)this)->_staraddr(pdata, addr, size,
        //getBlock
        [](File* self, u32* block, uint j, uint*, u64*, uint, void* buffer) {
            FS* fs = self->_fs;
            fs->_part->readaddr(block[j]*fs->_blockSize, buffer, fs->_blockSize);
            return false;
        },
        //doWork
        [](File* self, void* pdata, size_t blockId, size_t addr, size_t count) {
            FS* fs = self->_fs;
            u8*& data = *(u8**)pdata;
            fs->_part->readaddr(blockId*fs->_blockSize + addr, data, count);
            data += count;
        },
        //skip
        [](File*, int, uint*, u64*) {
            return true;
        },
        //prepareData
        [](File*, u32*, size_t, uint*, u64*, uint) {
            return false;
        }
    );
}

void File::_staraddr(void* data, u64 addr, size_t size, getBlockFunc getBlock, doWorkFunc doWork, skipFunc skip, prepareDataFunc prepareData) {
    u64 savedAddr = addr;
    size_t savedSize = size;
    uint blockSize = _fs->_blockSize;
    bool writeInode = false;
    bool writeBlock[3] = {false, false, false};

    u64 indirectSize[4];
    indirectSize[0] = 1;
    for(int i = 1; i < 4; ++i) {
        indirectSize[i] = (blockSize/4)*indirectSize[i-1];
    }

    uint is[3] = {0, 0, 0};
    //blocks[0] = the block of data.block
    //blocks[i+1] = the block at blocks[i][is[i]]
    u32* blocks[3];
    for(uint i = 0; i < 3; ++i) {
        blocks[i] = nullptr;
    }

    int i;
    for(i = 0; i < 12; ++i) {
        if(size == 0) {
            i = -1;
            goto end;
        }

        writeInode |= prepareData(this, (u32*)_data.block, i, is, indirectSize, i);
        if(addr >= blockSize) {
            addr -= blockSize;
        } else {
            uint count = min(blockSize - addr, (u64)size);
            doWork(this, data, _data.block[i], addr, count);
            size -= count;
            addr = 0;
        }
    }

    for(i = 0; i < 3; ++i) {
        if(size == 0) goto end;
        blocks[i] = (u32*)malloc(blockSize);

        bool canSkip = skip(this, 12+i, is, indirectSize);
        if(addr >= indirectSize[i+1]*blockSize) {
            if(canSkip) {
                addr -= indirectSize[i+1]*blockSize;
                continue;
            }
        }

        if(writeBlock[0]) {
            _fs->_part->writeaddr(_data.block[12+i-1]*blockSize, blocks[0], blockSize);
            writeBlock[0] = false;
        }

        writeInode |= getBlock(this, (u32*)_data.block, 12+i, is, indirectSize, 12+i, blocks[0]);

        if(canSkip) {
            for(int j = 0; j <= i; ++j) {
                is[j] += addr/(indirectSize[i-j]*blockSize);
                addr %= indirectSize[i-j]*blockSize;
            }
        }

        for(int j = 1; j <= i; ++j) {
            writeBlock[j-1] |= getBlock(this, blocks[j-1], is[j-1], is, indirectSize, 12+i, blocks[j]);
        }

        do {
            if(size == 0) goto end;

            writeBlock[i] |= prepareData(this, blocks[i], is[i], is, indirectSize, 12+i);
            if(addr < blockSize) {
                uint count = min(blockSize - addr, (u64)size);
                doWork(this, data, blocks[i][is[i]], addr, count);
                size -= count;
                addr = 0;
            } else {
                addr -= blockSize;
            }

            //increment `is` by `1` in base `blockSize/4` and update `blocks`
            for(int j = i; j >= 0; --j) {
                is[j] += 1;
                if(is[j] == blockSize/4) {
                    is[j] = 0;
                } else {
                    for(int k = j+1; k <= i; ++k) {
                        if(writeBlock[k]) {
                            _fs->_part->writeaddr(blocks[k-1][(is[k-1]+blockSize/4-1)%(blockSize/4)]*blockSize, blocks[k], blockSize);
                            writeBlock[k] = false;
                        }
                    }
                    for(int k = j+1; k <= i; ++k) {
                        writeBlock[k-1] |= getBlock(this, blocks[k-1], is[k-1], is, indirectSize, 12+i, blocks[k]);
                    }
                    break;
                }
            }
        } while(!(is[0] == 0 && is[1] == 0 && is[2] == 0));
    }

end:
    //write remaining blocks
    if(writeBlock[0]) {
        _fs->_part->writeaddr(_data.block[12+i]*blockSize, blocks[0], blockSize);
    }
    for(int j = 1; j <= i; ++j) {
        if(writeBlock[j]) {
            _fs->_part->writeaddr(blocks[j-1][is[j-1]]*blockSize, blocks[j], blockSize);
        }
    }

    //write inode
    if(true || writeInode) {
        _data.size = max(_data.size, (u32)(savedAddr+savedSize));
        _fs->writeInodeData(_inode, &_data);
    }

    for(int j = 0; j < 3; ++j) {
        if(blocks[j] != nullptr) {
            free(blocks[j]);
        }
    }
}

size_t File::getSize() const {
    return _data.size;
}

bool File::isInRAM() const {
    return false;
}

void* File::getData() {
    return nullptr;
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

