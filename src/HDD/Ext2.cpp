#include "Ext2.h"
#include "../IO/FrameBuffer.h"

namespace Ext2 {

FS::FS(Partition* part) : FileSystem(part) {
    _bgd = nullptr;
    _loadSuperBlock();
    assert(_sb.magic == 0xef53);
    loadBlockGroupDescriptor();
}

::Directory* FS::getRoot() {
    InodeData dat;
    getInodeData(2, &dat);
    return new Directory(dat, this);
}

void FS::getInodeData(uint inode, InodeData* res) {
    uint blockGroup  = (inode-1) / _sb.inodes_per_group;
    uint index       = (inode-1) % _sb.inodes_per_group;
    uint inodeTable = _bgd[blockGroup].inode_table;
    _part->readaddr(inodeTable*_blockSize + index*_sb.inode_size, res, sizeof(InodeData));
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

File::File(InodeData data, FS* fs) :
    _data(data), _fs(fs) {}

void File::writeaddr(u64 addr, const void* data, size_t size) {
    bsod("Ext2::File::writeaddr is not implemented :-(");
}

void File::readaddr(u64 addr, void* data, size_t size) const {
    Partition* part = _fs->_part;
    uint blockSize = _fs->_blockSize;
    u8* dat = (u8*)data;
    for(int i = 0; i < 12; ++i) {
        if(size == 0) return;

        if(addr >= blockSize) {
            addr -= blockSize;
        } else {
            uint count = min(blockSize - addr, (u64)size);
            part->readaddr(_data.block[i]*blockSize + addr, dat, count);
            dat += count;
            size -= count;
            addr = 0;
        }
    }

    u64 indirectSize[4];
    indirectSize[0] = blockSize;
    for(int i = 1; i < 4; ++i) {
        indirectSize[i] = (blockSize/4)*indirectSize[i-1];
    }

    uint is[3] = {0, 0, 0};
    //blocks[0] = the block of data.blocks
    //blocks[i+1] = the block at blocks[i][is[i]]
    u32* blocks[3];
    for(uint i = 0; i < 3; ++i) {
        blocks[i] = nullptr;
    }

    for(int i = 0; i < 3; ++i) {
        if(size == 0) goto end;
        blocks[i] = (u32*)malloc(blockSize);
        if(addr >= indirectSize[i+1]) {
            addr -= indirectSize[i+1];
            continue;
        }

        part->readaddr(_data.block[12+i]*blockSize, blocks[0], blockSize);
        for(int j = 0; j <= i; ++j) {
            is[j] += addr/indirectSize[i-j];
            addr %= indirectSize[i-j];
        }
        for(int j = 1; j <= i; ++j) {
            part->readaddr(blocks[j-1][is[j-1]]*blockSize, blocks[j], blockSize);
        }

        do {
            if(size == 0) goto end;
            uint count = min(blockSize - addr, (u64)size);
            part->readaddr(blocks[i][is[i]]*blockSize + addr, dat, count);
            dat += count;
            size -= count;
            addr = 0;

            //increment `is` by `1` in base `blockSize/4` and update `blocks`
            for(int j = i; j >= 0; --j) {
                is[j] += 1;
                if(is[j] == blockSize/4) {
                    is[j] = 0;
                } else {
                    for(int k = j+1; k <= i; ++k) {
                        part->readaddr(blocks[k-1][is[k-1]]*blockSize, blocks[k], blockSize);
                    }
                    break;
                }
            }
        } while(!(is[0] == 0 && is[1] == 0 && is[2] == 0));
    }

end:
    for(int i = 0; i < 3; ++i) {
        free(blocks[i]);
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


Directory::Directory(InodeData data, FS* fs) : File(data, fs) { }

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
        return new Directory(inodedat, _fs);
    } else if((inodedat.mode&0xE000) == FM_IFREG) {
        return new File(inodedat, _fs);
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

