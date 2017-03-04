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
    std::string _name;
    public:
    std::string getName(){return _name;}
    //virtual void setName(const std::string& name) = 0;
    //virtual Directory * getParent() = 0; // null => root directory
    virtual FileType getType();
    virtual Directory * dir() {return nullptr;};
};

class Directory : public virtual File {
public :
    virtual std::vector<std::string> getFilesName () = 0;
    FileType getType();
    virtual File * operator[](const std::string& name) = 0; // nullptr means it does not exists
    virtual Directory * dir() {return this;};
};

class FileSystem {
protected :
    Partition* _part;
public:
    explicit FileSystem (Partition * part);
    virtual Directory* getRoot() = 0;
    //virtual File* operator [] (const std::string& path) = 0; //return null when path is not valid
};





#endif
