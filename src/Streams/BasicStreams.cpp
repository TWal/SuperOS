#include "BasicStreams.h"
#include "../Random.h"


/*  ____  _                            _____
   / ___|| |_ _ __ ___  __ _ _ __ ___ |__  /___ _ __ ___
   \___ \| __| '__/ _ \/ _` | '_ ` _ \  / // _ \ '__/ _ \
    ___) | |_| | |  __/ (_| | | | | | |/ /|  __/ | | (_) |
   |____/ \__|_|  \___|\__,_|_| |_| |_/____\___|_|  \___/
*/


u64 StreamZero::getMask() const {
    return READABLE | WRITABLE | SEEKABLE | APPENDABLE;
}

size_t StreamZero::read(void* buf, size_t count) const {
    memset(buf, 0, count);
    return count;
}

bool StreamZero::eof() const {
    return false;
}

size_t StreamZero::write(const void*, size_t count) {
    return count;
}

size_t StreamZero::tell() const {
    return 0;
}

size_t StreamZero::seek(i64, mod) {
    return 0;
}


/*  ____  _                            _   _       _ _
   / ___|| |_ _ __ ___  __ _ _ __ ___ | \ | |_   _| | |
   \___ \| __| '__/ _ \/ _` | '_ ` _ \|  \| | | | | | |
    ___) | |_| | |  __/ (_| | | | | | | |\  | |_| | | |
   |____/ \__|_|  \___|\__,_|_| |_| |_|_| \_|\__,_|_|_|
*/

u64 StreamNull::getMask() const {
    return READABLE | WRITABLE | SEEKABLE | APPENDABLE;
}

size_t StreamNull::read(void*, size_t) const {
    return 0;
}

bool StreamNull::eof() const {
    return true;
}

size_t StreamNull::write(const void*, size_t count) {
    return count;
}

size_t StreamNull::tell() const {
    return 0;
}

size_t StreamNull::seek(i64, mod) {
    return 0;
}


/*  ____  _                            ____                 _
   / ___|| |_ _ __ ___  __ _ _ __ ___ |  _ \ __ _ _ __   __| | ___  _ __ ___
   \___ \| __| '__/ _ \/ _` | '_ ` _ \| |_) / _` | '_ \ / _` |/ _ \| '_ ` _ \
    ___) | |_| | |  __/ (_| | | | | | |  _ < (_| | | | | (_| | (_) | | | | | |
   |____/ \__|_|  \___|\__,_|_| |_| |_|_| \_\__,_|_| |_|\__,_|\___/|_| |_| |_|
*/

u64 StreamRandom::getMask() const {
    return READABLE | WRITABLE | SEEKABLE | APPENDABLE;
}

size_t StreamRandom::read(void* buf, size_t count) const {
    size_t nb = count/4;
    u32* buf32 = (u32*)buf;
    for(size_t i = 0; i < nb; ++i) {
        buf32[i] = random.rand();
    }
    if(count%4 == 0) return count;

    u32 rest = random.rand();
    u8* buf8 = (u8*)buf;

    for(size_t i = 4*nb; i < count; ++i) {
        buf8[i] = rest & 0xFF;
        rest >>= 8;
    }

    return count;
}

bool StreamRandom::eof() const {
    return false;
}

size_t StreamRandom::write(const void*, size_t count) {
    //(should add more entropy...)
    return count;
}

size_t StreamRandom::tell() const {
    return 0;
}

size_t StreamRandom::seek(i64, mod) {
    return 0;
}


