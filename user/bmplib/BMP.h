#ifndef BMP_H

#include <stdint.h>
#include <string>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef intptr_t iptr;
typedef uintptr_t uptr;

class BMP{
public:
    struct Color24{
        u8 B,G,R;
    };
    struct Color32 : Color24{
        u8 alpha;
        Color32(Color24 a) : Color24(a), alpha(0){}
        Color32& operator=(Color24 a){
            *(Color24*)this = a;
            alpha = 0;
            return *this;
        }
    };

private:

    Color32 *_buffer32;
    Color32 *_buffer32_2;
    Color24 *_buffer24;
    u32 _height;
    u32 _width;
    std::string _error;

    struct BMPHeader{
        char sign[2];
        u32 fileSize;
        u32 reserved;
        u32 offset;
        u32 headersize; // plus 0xa
        u32 width;
        u32 height;
        u16 planes;
        u16 depth;
        u32 compression;
        u32 size;
        // garbage
    }__attribute__((packed));

public:

    BMP() : _buffer32(nullptr), _buffer24(nullptr){}
    ~BMP(){
        if(_buffer24) delete _buffer24;
        if(_buffer32) delete _buffer32;
    }

    void load(std::string file);

    operator bool(){
        return _error == "";
    }

    std::string error(){
        return _error;
    }
    void to32();

    void draw(int windowfd);

    u32 w(){ return _width;}
    u32 h(){ return _height;}


};


#endif
