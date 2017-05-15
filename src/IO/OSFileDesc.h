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
   @brief list of OS wide Streams (POSIX file descriptors interfaces)

   stdin(0) & stdout(1) are for kernel command line.

   stderr(2) point to serial

   stdlog(3) sends kernel logs in as many place as possible.

   The number 0 (stdin) is currently disabled.

   see @read and @write
 */
extern std::vector<Stream*> OSStreams;

/**
   @brief Initialize IO Sytem after graphics : print all waiting kernel log to Stream str.

   From now all input-output pass through @ref Stream "Streams" in @ref OSStreams.

 */
void IOinit(Stream * str);

// OS posix read
extern "C" size_t read(int fd, void* buf, size_t count);

/**
   @brief OS write, see @ref OSStreams
 */
extern "C" size_t write(int fd, const void* buf, size_t count);



class SerialStream : public Stream{
public:
    u64 getMask() const {return Stream::WRITABLE | Stream::APPENDABLE;}
    size_t write(const void* buf,size_t count);
};


/// When true fd 2 is ignored before IOPostGraphicinit();
extern bool unitTest;



#endif
