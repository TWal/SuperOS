#ifndef PREFIXSTREAM
#define PREFIXSTREAM

#include "Stream.h"
#include <string>
#include "../log.h"

class PrefixStream : public Stream{
    Stream* _str;
    std::string _prefix;
    std::string _buffer;
public:
    inline PrefixStream(Stream* str, std::string prefix) :_str(str),_prefix(prefix){
    }

    u64 getMask()const {
        return _str->getMask() & (Stream::WRITABLE | Stream::APPENDABLE);
    }

    size_t write(const void * buf,size_t count){
        const char* buf2 = reinterpret_cast<const char*>(buf);
        for(size_t i = 0 ; i < count ; ++i){
            _buffer.push_back(*buf2);
            if(*buf2 == '\n'){
                _str->bwrite(_prefix.c_str(),_prefix.size());
                _str->bwrite(_buffer.c_str(),_buffer.size());
                _buffer.clear();
            }
            ++buf2;
        }
        return count;
    }

};


#endif
