#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include "../utility.h"

class FileDescriptor{
    u64 owners;
public :
    inline void grab(){++owners;}
    inline void drop(){--owners;if(!owners) delete this;}

    FileDescriptor();
    virtual ~FileDescriptor(){};

    // ----------------- Function checking------------------------
    enum{ // Functionality
        READABLE,WRITABLE,SEEKABLE
    };
    virtual u64 getMask() = 0;
    inline bool check(u8 val){
        return getMask() >> val & 1;
    }

    // -------------------Interacting----------------------
    virtual size_t read(void * buf,size_t count) =0;
    virtual size_t write(void * buf,size_t count) =0;
    virtual size_t lseek(void * buf,size_t count) =0;
};




#endif
