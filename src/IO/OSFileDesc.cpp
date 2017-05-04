#include "OSFileDesc.h"
#include "FrameBuffer.h"
#include "Serial.h"
#include "../Streams/OutMixStream.h"
#include "../Memory/Paging.h"

static bool init = false;

bool unitTest = false;

static char* physicalBuffer;

static char* const preGraphicBuffer = (char*) -0x90000000;

static uint pgBufferNbPages;

static uint posInPgBuffer = 0;


void IOPrePagingInit(char* phyBuffer, uint nbPages, uint pos){
    physicalBuffer = phyBuffer;
    pgBufferNbPages = nbPages;
    posInPgBuffer = pos;
}
void IOPreGraphicInit(){
    paging.createMapping((u64)physicalBuffer,preGraphicBuffer,pgBufferNbPages);
    physicalBuffer = nullptr;
}
void IOPostGraphicinit(){
    assert(OSStreams.size() >= 2);
    //OSStreams.push_back(nullptr);
    SerialStream * sers = new SerialStream();
    OSStreams[2] = sers;
    init = true;

    //OSStreams[1]->write(preGraphicBuffer,posInPgBuffer);
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
                ser.write(buf2[i]);
                /*     if(physicalBuffer) physicalBuffer[posInPgBuffer] = buf2[i];
                else preGraphicBuffer[posInPgBuffer] = buf2[i];
                ++posInPgBuffer;*/
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
    /*ser.write("\ndata :");*/
    const char* buf2 = (const char*)buf;
    for(size_t i = 0 ; i < count ; ++i){
        ser.write(buf2[i]);
    }

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
