#include "Font.h"
#include "../Memory/PageHeap.h"

namespace video{
    Font* Font::def = nullptr;

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
    void Font::defInit(u64 fontPhyPtr){
        def = pageHeap.alloc<Font>(fontPhyPtr,2);
        def->init();
    }

}
