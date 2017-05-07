#ifndef BYTESSTREAM_H
#define BYTESSTREAM_H

#include "../utility.h"
#include "../Bytes.h"
#include "Stream.h"

class BytesStream : public Stream{ // TODO test this
    Bytes* _data;
    u64 _addr;
public:
    u64 getMask(){return Stream::READABLE | Stream::WRITABLE | Stream::APPENDABLE;};
    explicit BytesStream(Bytes * data);
    size_t read(void* buf,size_t count);
    bool eof();
    size_t write(const void * buf,size_t count);
    size_t tell(){return _addr;}
    size_t seek(i64 count, mod mode);
};

#endif
