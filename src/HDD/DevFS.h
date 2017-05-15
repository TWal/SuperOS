#ifndef DEVFS_H
#define DEVFS_H

#include "FileSystem.h"
#include "RamFS.h"
#include "HardDrive.h"

namespace HDD {

class DevFS : public FileSystem {
    public:
        DevFS();
        virtual std::unique_ptr<::HDD::Directory> getRoot();
        void addHardDrive(const std::string& name, HDD* hdd);
    protected:
        RamFS::FS _fs;
};

} // end of namespace HDD

#endif

