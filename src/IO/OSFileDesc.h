#ifndef OSFILEDESC_H
#define OSFILEDESC_H

#include "../utility.h"
#include <vector>
#include "../Streams/Stream.h"


/**
   @file OSFileDesc.h
   @brief This file is about manipulating the OS global file descriptors,
   and providing clean IO for the OS
*/
/**
   @brief list of OS wide Streams

   Currently 1 is a compound of FrameBuffer and Serial and 2 is serial
   (see OutMixStream).

   The number 0 (stdin) is currently disabled.
 */
extern std::vector<Stream*> OSStreams;

/// Initialize OS file descriptors, no @ref printf (but @ref kprintf) before calling this function.
void IOinit();

// OS posix read
extern "C" size_t read(int fd, void* buf, size_t count);
extern "C" size_t write(int fd, const void* buf, size_t count);

class FBStream : public Stream{
public:
    u64 getMask() const {return Stream::WRITABLE;}
    size_t write(const void* buf,size_t count);
};

class SerialStream : public Stream{
public:
    u64 getMask() const {return Stream::WRITABLE;}
    size_t write(const void* buf,size_t count);
};


/// When true fd 2 is ignored before IOinit();
extern bool unitTest;



#endif
