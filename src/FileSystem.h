#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Partition.h"

class File : public HDDBytes {
};

class FileSystem {
protected :
    Partition* _part;
public:
    explicit FileSystem (Partition * part);
    //virtual File* operator [] (const char* path) = 0;// I need a HEAP !!!
};




#endif
