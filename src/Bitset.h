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



class Bitset{
    u64* _data;
    size_t _size;
public:
    Bitset(void* data,size_t size) : _data((u64*)data),_size(size){}
    void init(void* data,size_t size){
        _data = (u64*)data;
        _size = size;
    }
    bool get(size_t i) const {
        assert(i < _size);
        return _data[i/64] >> (i%64) & 1;
    }
    void set(size_t i){
        assert(i < _size);
        _data[i/64] |= (1L << (i%64));
    }
    void unset(size_t i){
        assert(i < _size);
        _data[i/64] &= ~(1L << (i%64));

    }
    void switchAddr(void* data){// data should be pointing to the same content.
        //this function is for example to switch from physical to virtual memory
        _data = (u64*)data;
    }
    void* getAddr() const {return _data;}
    __BoolRef operator[](size_t i){
        assert(i < _size);
        return __BoolRef(_data +i/64, i % 64);
    }
    size_t bsr()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = 0; i < _size/64; ++i) {
            if(_data[i]) {
                asm("bsr %1,%0" : "=r"(pos) : "r"(_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
    }
    // count ?
    size_t size() const {
        return _size;
    }
    void clear(){
        __builtin_memset(_data,0,_size/8);
    }
    void fill(){
        __builtin_memset(_data,-1,_size/8);
    }
};


#endif
