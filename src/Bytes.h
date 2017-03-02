#ifndef BYTES_H
#define BYTES_H

#include"utility.h"

class Bytes {
public:
    virtual void writeaddr (uptr addr,const void * data, size_t size) = 0;
    virtual void readaddr (uptr addr, void * data, size_t size) = 0;
    virtual size_t getSize() = 0; // 0 mean unknown or too big
    virtual bool isInRAM() = 0;
    virtual void* getData() = 0; // may fail if isInRAM() == false
};

class RAMBytes : public Bytes { // TODO implement it
};

class HDDBytes : public Bytes {

public :
    inline virtual u32 getLBASize(){
        return (getSize()+511L)/512L;
    }
    virtual void writelba (u32 LBA , const void* data, u32 nbsector) = 0;
    virtual void readlba (u32 LBA, void * data, u32 nbsector) = 0;

};

#endif
