#include "BytesStream.h"

BytesStream::BytesStream(Bytes * data): _data(data),_addr(0){
    assert(data->getSize() > 0);
}

size_t BytesStream::read(void* buf,size_t count){
    size_t toRead = max((size_t)0,min(count,_data->getSize()-_addr));
    _data->readaddr(_addr,buf,toRead);
    _addr+= toRead;
    return toRead;
}

bool BytesStream::eof(){
    return _addr == _data->getSize();
}

size_t BytesStream::write(const void * buf,size_t count){
    size_t toWrite = max((size_t)0,min(count,_data->getSize()-_addr));
    _data->writeaddr(_addr,buf,toWrite);
    _addr+= toWrite;
    return toWrite;
}

size_t BytesStream::seek(i64 count, mod mode){
    size_t toSet;
    switch(mode){
        case mod::BEG:
            toSet = count;
            break;
        case mod::CUR:
            toSet = _addr + count;
            break;
        case mod::END:
            toSet = _data->getSize() + count;
            break;
        default:
            return (size_t)-1;
    }
    if(toSet > _data->getSize()) return (size_t)-1;
    else return  _addr = toSet;
}