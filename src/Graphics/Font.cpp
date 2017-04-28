#include "Font.h"

namespace video{

    void Font::writeLine(uchar c,uint line, void* buf,uint fg, uint bg)const{
        uint* ibuf = reinterpret_cast<uint*>(buf);
        char bitset = data[c][line];
        for(int i = 0 ; i < 8 ; ++i){
            if(bitset & 128) *ibuf = fg;
            else *ibuf = bg;
            ++ibuf;
            bitset <<= 1;
        }
    }

}
