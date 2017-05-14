#ifndef PIPESTREAM_H
#define PIPESTREAM_H

#include "../utility.h"
#include <deque>
#include "Stream.h"

class PipeStreamOut;

class PipeStream : public Stream {
    friend PipeStreamOut;
    public:
        PipeStream();
        virtual ~PipeStream();
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;
        virtual size_t write(const void* buf, size_t count);
        void decrRef();
        void sendEof();

    private:
        std::deque<char> _deque;
        PipeStreamOut* _out;
        bool _eof;
        u8 _nbRef;
};

class PipeStreamIn : public Stream {
    public:
        PipeStreamIn(PipeStream* ps);
        virtual ~PipeStreamIn();
        virtual u64 getMask() const;
        virtual size_t write(const void* buf, size_t count);

    private:
        PipeStream* _ps;
};

class PipeStreamOut : public Stream {
    public:
        PipeStreamOut(PipeStream* ps);
        virtual ~PipeStreamOut();
        virtual u64 getMask() const;
        virtual size_t read(void* buf, size_t count);
        virtual bool eof() const;

    private:
        PipeStream* _ps;
};

#endif

