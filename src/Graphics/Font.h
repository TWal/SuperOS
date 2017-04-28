#ifndef FONT_H
#define FONT_H

#include"../utility.h"
namespace video{

    using FontChar = char[16];

    class Font{
        u16 magic; // must be 0x0436
        u8 mode;
        u8 size; // must be 16.
        FontChar data[256];
    public:
        void init(){
            assert(magic == 0x0436);
            assert(size == 16);
        }
        void writeLine(uchar c,uint line, void* buf,uint fg, uint bg) const ;
    };
};

#endif
