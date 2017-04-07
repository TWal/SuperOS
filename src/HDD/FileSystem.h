#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Partition.h"
#include <vector>
#include <string>

//TODO: move this in libc?
struct dirent {
    u32 d_ino;
    char d_name[256];
};

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
    FileType getType();
    virtual File* operator[](const std::string& name) = 0; // nullptr means it does not exists
    virtual Directory* dir() {return this;};

    virtual void* open() = 0;
    virtual dirent* read(void* d) = 0;
    virtual long int tell(void* d) = 0;
    virtual void seek(void* d, long int loc) = 0;
    virtual void close(void* d) = 0;
};

class FileSystem {
protected :
    Partition* _part;
public:
    explicit FileSystem (Partition * part);
    virtual Directory* getRoot() = 0;
};





#endif
