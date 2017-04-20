#ifndef STREAM_H
#define STREAM_H

#include"../utility.h"

class Stream{
    enum{ // Functionality
        READABLE,WRITABLE,SEEKABLE
    };
    virtual u64 getMask() = 0;
    inline bool check(u8 val){
        return (getMask() >> val) & 1;
    }

    // -------------------Interacting----------------------
    virtual size_t read(void * buf,size_t count) =0;
    virtual size_t write(void * buf,size_t count) =0;

    virtual size_t tell() = 0;
    enum class mod{BEG,CUR,END};
    virtual size_t seek(size_t count, mod mode) =0;
};

#endif
