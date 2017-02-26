#ifndef BYTES_H
#define BYTES_H

#include"utility.h"

class Bytes {
public:
    virtual void writeaddr (ulint addr,const void * data, uint size) = 0;
    virtual void readaddr (ulint addr, void * data, uint size) = 0;
    virtual ulint getSize() = 0; // 0 mean unknown or too big
    virtual bool isInRAM() = 0;
    virtual void* getData() = 0; // may fail if isInRAM() == false
};

class RAMBytes : public Bytes { // TODO implement it
};

class HDDBytes : public Bytes {

public :
    inline virtual uint getLBASize(){
        return (getSize()+511L)/512L;
    }
    virtual void writelba (ulint LBA , const void* data, uint nbsector) = 0;
    virtual void readlba (ulint LBA, void * data, uint nbsector) = 0;

};

#endif
