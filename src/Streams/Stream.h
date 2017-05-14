#ifndef STREAM_H
#define STREAM_H

#include "../utility.h"
#include "../Processes/Waiting.h"
#include <stdio.h>

/**
   @brief Represent any stream of data, It is the data pointed o by a file descriptor

   A Stream can represent a File, the console(/dev/tty), a pipe, ...
 */

class Stream : public Waitable{
public:
    Stream() :Waitable(){};
    enum{ // Functionality
        READABLE = 1, WRITABLE = 2, SEEKABLE = 4, APPENDABLE = 8, WAITABLE = 16,
    };
    /// get the functionalities of this Stream.
    virtual u64 getMask() const = 0;
    /// Check for a specific functionality in the stream
    inline bool check(u8 val) const {
        return getMask() & val;
    }
    virtual ~Stream(){};

    // -------------------Interacting----------------------
    /**
       @brief Read as mush character as possible right now.

       If return value is 0, it does not mean @ref eof. It can be just that there is
       nothing to read right now. If this is the case, (read return 0 and eof return
       false), the flags WAITABLE must be set. (sysread will make an assert)

       A object of class Waiting can be hooked to the stream with
       value @ref WAITREAD to wait for something to read.

       UB if @ref READABLE is not set.
    */
    virtual size_t read(void* buf, size_t count) {(void)buf; (void)count;return -1;};

    /// Check if there is nothing more to read.
    virtual bool eof() const {return true;};

    /**
       @brief Try to write count byte from buf in the stream.

       If the stream is full or any other reason, write can write less than that
       (even 0) and return the number of bytes really written.

       This function is UB if @ref WRITABLE is not set.

       If @ref APPENDABLE is set, write can write after the end and should never
       return 0 except in case of error.

       @todo Proper error handling.
     */
    virtual size_t write(const void* buf, size_t count) {(void)buf; (void)count;return -1;};

    /**
       @brief Blocking write: Does not needs to be overloaded,
       does a complete write by waiting until all is written

       return is either a value <=0 : error but no way to know
       how many has been written.
       or count.
     */
    size_t bwrite(const void * buf,size_t count){
        const char* buf2 = (const char*)buf;
        size_t count_save = count;
        while(count){
            size_t res = write(buf2,count);
            if(i64(res) <= 0) return res;
            count -= res;
            buf2 += res;
        }
        return count_save;
    }

    /// Get the curent position if fd is @ref SEEKABLE.
    virtual size_t tell() const {return -1;};
    enum class mod{BEG,CUR,END};
    /// Set the curent position if fd is @ref SEEKABLE.
    virtual size_t seek(i64 count, mod mode){(void)mode; (void)count;return -1;};
};

#endif
