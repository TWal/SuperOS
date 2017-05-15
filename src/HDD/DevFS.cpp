#include "DevFS.h"
#include "../Streams/BasicStreams.h"

namespace HDD {

DevFS::DevFS() {
    u16 mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    CharacterDevice* zero = _fs.getNewCharacterDevice(new StreamZero, 0, 0, mode);
    CharacterDevice* null = _fs.getNewCharacterDevice(new StreamNull, 0, 0, mode);
    CharacterDevice* random = _fs.getNewCharacterDevice(new StreamRandom, 0, 0, mode);

    std::unique_ptr<Directory> d = _fs.getRoot();
    d->addEntry("zero", zero);
    d->addEntry("null", null);
    d->addEntry("random", random);
}

std::unique_ptr<::HDD::Directory> DevFS::getRoot() {
    return _fs.getRoot();
}

void DevFS::addHardDrive(const std::string& name, HDD* hdd) {
    const std::string numbers[4] = {"1", "2", "3", "4"};
    u16 mode = S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    RamFS::BlockDevice* block = _fs.getNewBlockDevice(hdd, 0, 0, mode);
    _fs.getRoot()->addEntry(name, block);
    for(u8 i = 0; i < 4; ++i) {
        PartitionTableEntry p = (*hdd)[i+1];
        if(p.size == 0) continue;
        Partition* part = new Partition(hdd, p);
        RamFS::BlockDevice* block = _fs.getNewBlockDevice(part, 0, 0, mode);
        _fs.getRoot()->addEntry(name + numbers[i], block);
    }
}

} // end of namespace HDD

