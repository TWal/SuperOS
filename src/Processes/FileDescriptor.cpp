#include "FileDescriptor.h"
#include "../Streams/BytesStream.h"

FileDescriptor::FileDescriptor() : _owners(nullptr),_str(nullptr),_type(EMPTY){
}

FileDescriptor::FileDescriptor(std::unique_ptr<Stream>&& str)
    : _owners(new u64(0)), _str(new std::unique_ptr<Stream>(std::move(str))), _type(STREAM){
    *_owners = 1;
}

FileDescriptor::FileDescriptor(std::unique_ptr<HDD::Directory>&& dir)
    : _owners(new u64(0)), _dir(new std::unique_ptr<HDD::Directory>(std::move(dir))), _type(DIRECTORY){
    *_owners = 1 ;
}

FileDescriptor::FileDescriptor(video::GraphWindow* win)
    :_owners(new u64(0)), _win(win), _type(GWINDOW){
    *_owners = 1;
    std::unique_ptr<Bytes> winBytes(win);
    winBytes.dontDelete();
    _str = new std::unique_ptr<Stream>(new BytesStream(std::move(winBytes)));
}
FileDescriptor::FileDescriptor(video::TextWindow* win)
    :_owners(new u64(0)), _str(new std::unique_ptr<Stream>(win)), _win(win), _type(TWINDOW){
    *_owners = 1;
}

FileDescriptor::~FileDescriptor(){
    drop();
}

FileDescriptor::FileDescriptor(const FileDescriptor& other):
    _owners(other._owners), _str(other._str), _type(other._type),
    _mask(other._mask){
    if(_owners)(*_owners)++;

    if(_type == GWINDOW or _type == TWINDOW) _win = other._win;
    if(_type == DIRECTORY) _dir = other._dir;
}

FileDescriptor& FileDescriptor::operator=(const FileDescriptor& fd){
    drop();
    return *(new (this) FileDescriptor(fd));
}

void FileDescriptor::drop(){
    if(!_owners) return;
    (*_owners)--;
    if(!*_owners) free();
}

void FileDescriptor::free(){
    assert(_owners and *_owners == 0);
    delete _owners;
    delete _str;
    switch(_type){
        case EMPTY:
        case TWINDOW: // the stream and the window are part of the same virtual class.
        case STREAM:
            return;
        case DIRECTORY:
            delete _dir;
            return;
        case GWINDOW:
            delete _win;
            return;
    }
}
