#include "OSFileDesc.h"
#include "FrameBuffer.h"
#include "Serial.h"
#include "../Streams/OutMixStream.h"

bool init = false;

bool unitTest = false;

void IOinit(){
    OSStreams.push_back(nullptr);
    FBStream * fbs = new FBStream();
    SerialStream * sers = new SerialStream();
    OSStreams.push_back(fbs);
    OSStreams.push_back(sers);
    init = true;
}

size_t read(int fd, void* buf, size_t count){
    assert((size_t)fd < OSStreams.size());
    assert(OSStreams[fd] && OSStreams[fd]->check(Stream::READABLE));
    return OSStreams[fd]->read(buf,count);
}
size_t write(int fd, const void* buf, size_t count){
    if(!init){
        if(fd == 1){
            const char* buf2 = (const char*)buf;
            for(size_t i = 0 ; i < count ; ++i){
                fb.putc(buf2[i]);
            }
            return count;
        }
        if(fd == 2){
            if (unitTest) return count; // silently ignore on unit test;
            const char* buf2 = (const char*)buf;
            for(size_t i = 0 ; i < count ; ++i){
                ser.write(buf2[i]);
            }
            return count;
        }
        bsod("Use of file descriptor 0 or >= 3 before IOinit.");
    }
    assert((size_t)fd < OSStreams.size());
    assert(OSStreams[fd] && OSStreams[fd]->check(Stream::WRITABLE));
    return OSStreams[fd]->write(buf,count);
}

std::vector<Stream*> OSStreams;

size_t FBStream::write(const void* buf,size_t count){
    const char* buf2 = (const char*)buf;
    for(size_t i = 0 ; i < count ; ++i){
        fb.putc(buf2[i]);
    }
    return count;
}


size_t SerialStream::write(const void* buf,size_t count){
    const char* buf2 = (const char*)buf;
    for(size_t i = 0 ; i < count ; ++i){
        ser.write(buf2[i]);
    }
    return count;
}
