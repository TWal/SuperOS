#include "FileSystem.h"

namespace HDD {

u32 File::getInode() const {
    stat st;
    getStats(&st);
    return st.st_ino;
}

FileType RegularFile::getType() const {
    return FileType::RegularFile;
}

FileType Directory::getType() const {
    return FileType::Directory;
}

FileType BlockDevice::getType() const {
    return FileType::BlockDevice;
}

FileType CharacterDevice::getType() const {
    return FileType::CharacterDevice;
}

FileSystem::FileSystem(Partition* part) : _part(part){
}

bool Directory::isEmpty() {
    void* d = open();
    dirent* dir;
    u32 count = 0;
    bool res = true;
    while((dir = read(d)) != nullptr) {
        count += 1;
        if(count > 2) {
            res = false;
            break;
        }
        char* name = dir->d_name;
        if(!(name[0] == '.' && ((name[1] == '.' && name[2] == '\0') || name[1] == '\0'))) {
            res = false;
            break;
        }
    }
    close(d);
    return res;
}

} //end of namespace HDD

