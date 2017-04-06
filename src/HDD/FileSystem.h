#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Partition.h"
#include <vector>
#include <string>

class Directory;

enum class FileType {
    File, Directory
};

class File : public HDDBytes {
    public:
    virtual FileType getType();

    //undefined behavior if the file is a directory
    //no guarantees when it augments the size on what is after the old size
    virtual void resize(size_t size) = 0;
    virtual Directory* dir() {return nullptr;};
};

class Directory : public virtual File {
public :
    virtual std::vector<std::string> getFilesName () = 0;
    FileType getType();
    virtual File* operator[](const std::string& name) = 0; // nullptr means it does not exists
    virtual Directory* dir() {return this;};
};

class FileSystem {
protected :
    Partition* _part;
public:
    explicit FileSystem (Partition * part);
    virtual Directory* getRoot() = 0;
};





#endif
