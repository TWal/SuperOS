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

/**
   @brief Initialize IO system before paging

   Logs will be written to the physical buffer waiting for a graphical console to display.
   Logs will still be written immediately to Serial port
   @todo add a file output.
*/
void IOPrePagingInit(char* phyBuffer, uint nbPages, uint pos);

/**
   @brief Initialize IO System after paging but before graphics

   System Console needs heap so there is an area between paging and heap.
   The temporary buffer is mapped to -2.25G (cf @ref mappings).
 */
void IOPreGraphicInit();

/**
   @brief Initialize IO Sytem after graphics : print all waiting data to fd 1
   in @ref OSStreams.

   From now all input-output pass through @ref Stream "Streams" in @ref OSStreams.

   @todo handle graphical Output

 */
void IOPostGraphicinit();

// OS posix read
extern "C" size_t read(int fd, void* buf, size_t count);

/**
   @brief OS write, currentlty fd 1 point to all output, fd 2 to serial only
 */
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


/// When true fd 2 is ignored before IOPostGraphicinit();
extern bool unitTest;



#endif
