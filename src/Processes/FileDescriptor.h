#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include "../utility.h"
#include "../Streams/Stream.h"

class FileDescriptor{
    u64 _owners;
    Stream* _str;
public :
    inline void grab(){++_owners;}
    inline void drop(){--_owners;if(!_owners) delete this;}

    FileDescriptor(Stream* s);
    Stream* str(){return _str;}
};




#endif
