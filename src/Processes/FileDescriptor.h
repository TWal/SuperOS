#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include "../utility.h"
#include "../Streams/Stream.h"

class FileDescriptor{
    u64* _owners;
    Stream* _str;
    void free();
    void drop();
public :
    FileDescriptor();
    /**
       @brief Create a file descriptor on a stream.

       Take ownership of the pointer str.
     */
    FileDescriptor(Stream* str);
    ~FileDescriptor();
    FileDescriptor(const FileDescriptor& other);

    FileDescriptor& operator=(const FileDescriptor& fd);

    Stream* operator->(){assert(_str); return _str;}
    bool empty(){return !_str;}
};




#endif
