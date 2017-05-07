#ifndef BYTES_H
#define BYTES_H

#include"utility.h"

/**
    @brief Represents a sequence of bytes
*/
class Bytes {
public:
    virtual ~Bytes(){}
    /// Write `data` of size `size` at the address `addr`
    virtual void writeaddr(u64 addr, const void* data, size_t size) = 0;
    /// Read `data` of size `size` at the address `addr`
    virtual void readaddr(u64 addr, void* data, size_t size) const = 0;
    /// Get the size of this sequence. 0 means "unknown"
    virtual size_t getSize() const = 0; // 0 mean unknown or too big
};

/**
    @brief Represents a sequence of bytes on a hard drive
    @todo: Is it really useful?
*/
class HDDBytes : public Bytes {
public :
    inline virtual u32 getLBASize() const{
        return (getSize()+511L)/512L;
    }
    ///Same as writeaddr(LBA*512, data, nbsector*512)
    virtual void writelba(u32 LBA , const void* data, u32 nbsector);
    ///Same as readaddr(LBA*512, data, nbsector*512)
    virtual void readlba(u32 LBA, void* data, u32 nbsector) const;

};

#endif
