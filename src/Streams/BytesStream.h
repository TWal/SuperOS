#ifndef BYTESSTREAM_H
#define BYTESSTREAM_H

#include "../utility.h"
#include "../Bytes.h"
#include "Stream.h"

class BytesStream : public Stream {
    std::unique_ptr<Bytes> _data;
    u64 _addr;
public:
    explicit BytesStream(std::unique_ptr<Bytes> data);
    u64 getMask() const {return Stream::READABLE | Stream::WRITABLE
            | Stream::APPENDABLE | Stream::SEEKABLE;};
    size_t read(void* buf,size_t count);
    bool eof();
    size_t write(const void * buf,size_t count);
    size_t tell() const {return _addr;}
    size_t seek(i64 count, mod mode);
    size_t end() const {return _data->getSize();}
};

#endif
