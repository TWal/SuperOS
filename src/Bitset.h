#ifndef BITSET_H
#define BITSET_H

#include"utility.h"

class __BoolRef{
    u64* _place;
    u8 _pos;
public:
    inline __BoolRef(u64* place,u8 pos){
        assert(pos < 64);
        _place = place;
        _pos = pos;
    }
    inline __BoolRef operator=(bool b){
        if(b){
            *_place |= (1L <<_pos);
        }
        else{
            *_place &= ~(1L << _pos);
        }
        return *this;
    }
    inline operator bool(){
        return (*_place >> _pos) & 1;
    }
};

/**
   @brief This class provide a bitset interface on arbitrary data.

   The Bitset object does not own the underlying data, it is just an interface.
 */
class Bitset{
    u64* _data;
    size_t _size;
public:
    /// Create empty bitset, all other operation are UB until @ref init.
    Bitset() : _data(nullptr), _size(0) {}

    /// Create bitset manipulating data with size (in bits) size.
    Bitset(void* data,size_t size) : _data((u64*)data), _size(size){}

    /// Initialize a bitset to data with size size.
    void init(void* data,size_t size){
        _data = (u64*)data;
        _size = size;
    }

    /// Access bit i
    bool get(size_t i) const {
        assert(i < _size);
        return _data[i/64] >> (i%64) & 1;
    }

    /// Set the bit i to 1
    void set(size_t i){
        assert(i < _size);
        _data[i/64] |= (1L << (i%64));
    }

    /// Set range of bit starting at i of length len to 1
    void set(size_t i,size_t len){
        assert(i + len <= _size);
        size_t mod = i % 64;
        if(mod){
            if(len <= 64 - mod){
                _data[i/64] |= (((1ull << (len)) -1ull) << mod);
                return;
            }
            _data[i/64] |= u64(-1ull) - ((1ull << i) - 1ull);
        }
        i = alignup(i,64);
        len -= (64 -mod);
        while(len >= 64){
            _data[i/64] = u64(-1ull);
            i += 64;
            len -= 64;
        }
        _data[i/64] |= ((1ull << len) -1ull);
    }

    /// unset the bit i
    void unset(size_t i){
        assert(i < _size);
        _data[i/64] &= ~(1L << (i%64));

    }

    /// Unset range of bit starting at i of length len to 1
    void unset(size_t i,size_t len){
        assert(i + len <= _size);
        size_t mod = i % 64;
        if(mod){
            if(len <= 64 - mod){
                _data[i/64] &= ~(((1ull << (len)) -1ull) << mod);
                return;
            }
            _data[i/64] &= ~(u64(-1ull) - ((1ull << i) - 1ull));
        }
        i = alignup(i,64);
        len -= (64 -mod);
        while(len >= 64){
            _data[i/64] = 0;
            i += 64;
            len -= 64;
        }
        _data[i/64] &= ~((1ull << len) -1ull);
    }

    /**
       @brief Switch to a new data buffer (same size)

       Data should be pointing to the same content.

       This function is for example to switch from physical to virtual memory
     */
    void switchAddr(void* data){
        _data = (u64*)data;
    }

    /// Returns the pointer to manipulated bitset
    void* getAddr() const {return _data;}

    /// Access to specific boolean.
    __BoolRef operator[](size_t i){
        assert(i < _size);
        return __BoolRef(_data +i/64, i % 64);
    }

    /// Get the rightmost bit set.
    size_t bsr()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = _size/64; i != (size_t)(-1); --i) {
            if(_data[i]) {
                asm("bsr %1,%0" : "=r"(pos) : "r"(_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
    }

    /// Get the rightmost bit unset
    size_t usr() const{
        size_t i;
        u64 pos = 0;
        for(i = _size/64 -1; i != (size_t)(-1); --i) {
            if(_data[i] != u64(-1)) {
                asm("bsr %1,%0" : "=r"(pos) : "r"(~_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
    }

    /// Get the leftmost bit set.
    size_t bsf()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = 0; i < _size/64; ++i) {
            if(_data[i]) {
                asm("bsf %1,%0" : "=r"(pos) : "r"(_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
    }

    /// Get nb bits consecutive (not really the left most because of implem).
    size_t largeBsf(int nb)const {
        assert(nb < 64-8);
        size_t i;
        u64 pos = 0;
        for(i = 0; i < (_size+63)/64; ++i) {
            u64 data = -1;
            for(int j = 0 ; j < nb ; ++j){
                //printf("test %d %llx\n",j,data);
                data &= _data[i] << j;
            }
            //printf("test %llx\n",data);
            if(data) {
                asm("bsf %1,%0" : "=r"(pos) : "r"(data));
                size_t res = pos +1 - nb + i *64;
                if(res > _size - nb){
                    return -1;
                }
                return res;
            }
        }
        return (-1);
    }

    // count ?

    /// Get size of bitset
    size_t size() const {
        return _size;
    }

    /// Clear the bitset to 0.
    void clear(){
        __builtin_memset(_data,0,_size/8);
    }

    /// Fill the bitset to 1.
    void fill(){
        __builtin_memset(_data,-1,_size/8);
    }
};


#endif
