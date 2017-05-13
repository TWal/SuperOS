#include "PipeStream.h"


/*  ____  _            ____  _
   |  _ \(_)_ __   ___/ ___|| |_ _ __ ___  __ _ _ __ ___
   | |_) | | '_ \ / _ \___ \| __| '__/ _ \/ _` | '_ ` _ \
   |  __/| | |_) |  __/___) | |_| | |  __/ (_| | | | | | |
   |_|   |_| .__/ \___|____/ \__|_|  \___|\__,_|_| |_| |_|
           |_|
*/

PipeStream::PipeStream() :
    _deque(), _eof(false), _nbRef(2) {
}

PipeStream::~PipeStream() {
}

u64 PipeStream::getMask() const {
    return READABLE | WRITABLE | APPENDABLE;
}

size_t PipeStream::read(void* buf, size_t count) {
    char* cbuf = (char*)buf;
    size_t res = 0;

    while(res < count && !_deque.empty()) {
        cbuf[res] = _deque.front();
        _deque.pop_front();
        ++res;
    }

    return res;
}

bool PipeStream::eof() const {
    return _eof;
}

size_t PipeStream::write(const void* buf, size_t count) {
    const char* cbuf = (const char*)buf;
    for(size_t i = 0; i < count; ++i) {
        _deque.push_front(cbuf[i]);
    }
    return count;
}

void PipeStream::decrRef() {
    _nbRef -= 1;
    if(_nbRef == 0) {
        delete this;
    }
}

void PipeStream::sendEof() {
    _eof = true;
}


/*  ____  _            ____  _                            ___
   |  _ \(_)_ __   ___/ ___|| |_ _ __ ___  __ _ _ __ ___ |_ _|_ __
   | |_) | | '_ \ / _ \___ \| __| '__/ _ \/ _` | '_ ` _ \ | || '_ \
   |  __/| | |_) |  __/___) | |_| | |  __/ (_| | | | | | || || | | |
   |_|   |_| .__/ \___|____/ \__|_|  \___|\__,_|_| |_| |_|___|_| |_|
           |_|
*/

PipeStreamIn::PipeStreamIn(PipeStream* ps) :
    _ps(ps) {
}

PipeStreamIn::~PipeStreamIn() {
    _ps->sendEof();
    _ps->decrRef();
}

u64 PipeStreamIn::getMask() const {
    return WRITABLE | APPENDABLE;
}

size_t PipeStreamIn::write(const void* buf, size_t count) {
    return _ps->write(buf, count);
}



/*  ____  _            ____  _                             ___        _
   |  _ \(_)_ __   ___/ ___|| |_ _ __ ___  __ _ _ __ ___  / _ \ _   _| |_
   | |_) | | '_ \ / _ \___ \| __| '__/ _ \/ _` | '_ ` _ \| | | | | | | __|
   |  __/| | |_) |  __/___) | |_| | |  __/ (_| | | | | | | |_| | |_| | |_
   |_|   |_| .__/ \___|____/ \__|_|  \___|\__,_|_| |_| |_|\___/ \__,_|\__|
           |_|
*/

PipeStreamOut::PipeStreamOut(PipeStream* ps) :
    _ps(ps) {}

PipeStreamOut::~PipeStreamOut() {
    _ps->decrRef();
}

u64 PipeStreamOut::getMask() const {
    return READABLE;
}

size_t PipeStreamOut::read(void* buf, size_t count) {
    return _ps->read(buf, count);
}

bool PipeStreamOut::eof() const {
    return _ps->eof();
}


