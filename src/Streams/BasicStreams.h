#ifndef BASICSTREAMS_H
#define BASICSTREAMS_H

#include "Stream.h"

class StreamZero : public Stream {
    public:
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        virtual size_t tell() const;
        virtual size_t seek(i64 count, mod mode);
};

class StreamNull : public Stream {
    public:
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        virtual size_t tell() const;
        virtual size_t seek(i64 count, mod mode);
};

class StreamRandom : public Stream {
    public:
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        virtual size_t tell() const;
        virtual size_t seek(i64 count, mod mode);
};

#endif

