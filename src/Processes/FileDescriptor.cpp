#include "FileDescriptor.h"

FileDescriptor::FileDescriptor() : _owners(nullptr),_str(nullptr){
}

FileDescriptor::FileDescriptor(Stream* str): _owners(new u64(0)),
                                             _str(str){
    *_owners = 1;
}

FileDescriptor::~FileDescriptor(){
    drop();
}

FileDescriptor::FileDescriptor(const FileDescriptor& other):
    _owners(other._owners),_str(other._str){
    if(_owners)(*_owners)++;
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
    assert(_owners && _owners == 0);
    delete _owners;
    delete _str;
}

