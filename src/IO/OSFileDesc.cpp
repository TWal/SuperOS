#include "OSFileDesc.h"
#include "FrameBuffer.h"
#include "Serial.h"
#include "../Streams/OutMixStream.h"
#include "../Memory/Paging.h"
#include "../log.h"
#include <errno.h>


static bool init = false;

bool unitTest = false;

char preGraphicBuffer[KERNELBUFFER * 0x1000];

static uint posInBuffer = 0;

void IOinit(Stream* str){
    assert(OSStreams.size() >= 4);
    init = true;
    // send temp buffer in 3
    str->bwrite(preGraphicBuffer,posInBuffer);
}

size_t read(int fd, void* buf, size_t count){
    assert((size_t)fd < OSStreams.size());
    assert(OSStreams[fd] && OSStreams[fd]->check(Stream::READABLE));
    int res  = OSStreams[fd]->read(buf,count);
    //debug("Read returns %d",res);
    if(res < 0){
        errno = -res;
        return -1;
    }
    return res;
}
size_t write(int fd, const void* buf, size_t count){
    if(!init){
        if(fd == 3){
            const char* buf2 = (const char*)buf;
            for(size_t i = 0 ; i < count ; ++i){
                ser.write(buf2[i]);
                assert(posInBuffer < KERNELBUFFER * 0x1000);
                preGraphicBuffer[posInBuffer] = buf2[i];
                ++posInBuffer;
            }
            return count;
        }
        if(fd == 2 or fd == 1){
            if (unitTest) return count; // silently ignore on unit test;
            const char* buf2 = (const char*)buf;
            for(size_t i = 0 ; i < count ; ++i){
                ser.write(buf2[i]);
            }
            return count;
        }
        bsod("Use of file descriptor 0,1 or > 3 before IOinit.");
    }
    /*ser.write("\ndata :");*/
    /*const char* buf2 = (const char*)buf;
    for(size_t i = 0 ; i < count ; ++i){
        ser.write(buf2[i]);
        }*/

    assert((size_t)fd < OSStreams.size());
    assert(OSStreams[fd] && OSStreams[fd]->check(Stream::WRITABLE));
    //return count;
    //ser.write("written\n");
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
