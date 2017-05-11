#include "Screen.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"
#include <string.h>
#include "../log.h"
#include <emmintrin.h>

namespace video {
    Color24 Color24::white{255,255,255};
    Color24 Color24::black{0,0,0};
    Color24 Color24::red{0,0,255};
    Color24 Color24::blue{255,100,40};
    Color24 Color24::green{0,255,0};
    Color24 Color24::yellow{0,255,255};
    Color24 Color24::magenta{255,0,255};
    Color24 Color24::cyan{255,255,0};
    Color24 Color24::lred{0,0,187};
    Color24 Color24::lblue{187,50,20};
    Color24 Color24::lgreen{0,128,0};
    Color24 Color24::lyellow{0,187,187};
    Color24 Color24::lmagenta{187,30,187};
    Color24 Color24::lcyan{187,187,0};
    Color24 Color24::lwhite{187,187,187};

    char* const Screen::VGAbuffer = (char*)-0x140000000ll;
    Color* const Screen::buffer= (Color*)-0x120000000ll;
    void Screen::init(GraphicalParam* gp){
        info(Screenl,"Screen init with: %p",gp);
        Xsize = gp->Xsize;
        Ysize = gp->Ysize;
        pitch = gp->pitch;
        debug(Screenl,"Pitch :%d",pitch);


        // VRAM mapping
        u32 VRAMsize = Ysize*pitch;
        debug(Screenl,"VGAbuffer %p, VRAMsize %d",VGAbuffer,VRAMsize);
        paging.createMapping(gp->physptr,VGAbuffer,(int)VRAMsize/0x1000,true);

        // RAM mapping
        u32 RAMsize = Ysize*Xsize*4;
        debug(Screenl,"buffer %p, RAMsize %d",buffer,RAMsize);
        for(u32 i =0 ; i < RAMsize / 0x1000 ; ++i){
            paging.createMapping(physmemalloc.alloc(),buffer + i * (0x1000 / 4));
        }
        clear();
        /*for(int i = 0 ; i < 766 * 1024 ; ++i){
            buffer[i] = Color::white;
        }
        send();*/
        _OK = true;

    }
    void Screen::set(uint x, uint y, Color c){
        assert(x < Xsize);
        assert(y < Ysize);
        //printf("Set %d,%d\n",x,y);
        buffer[x + y * Xsize] = c;
    }

    //adapted from https://github.com/xbmc/xbmc/blob/master/xbmc/utils/win32/memcpy_sse2.h
    static inline void* memcpy_aligned(void* dst, const void* src, size_t size) {
        size_t i;
        __m128i xmm1, xmm2, xmm3, xmm4;

        if((((size_t)(src) | (size_t)(dst)) & 0xF)) {
            return memcpy(dst, src, size);
        }

        uint8_t* d = (uint8_t*)(dst);
        uint8_t* s = (uint8_t*)(src);

        for(i = 0; i < size - 63; i += 64) {
            xmm1 = _mm_load_si128((__m128i*)(s + i +  0));
            xmm2 = _mm_load_si128((__m128i*)(s + i + 16));
            xmm3 = _mm_load_si128((__m128i*)(s + i + 32));
            xmm4 = _mm_load_si128((__m128i*)(s + i + 48));

            _mm_stream_si128((__m128i*)(d + i +  0), xmm1);
            _mm_stream_si128((__m128i*)(d + i + 16), xmm2);
            _mm_stream_si128((__m128i*)(d + i + 32), xmm3);
            _mm_stream_si128((__m128i*)(d + i + 48), xmm4);
        }

        for(; i < size; i += 16) {
            xmm1 = _mm_load_si128((__m128i*)(s + i));
            _mm_stream_si128((__m128i*)(d + i), xmm1);
        }

        return dst;
    }

    void Screen::send(){
        //printf("Screen Update");
        for(int i = 0 ; i < Ysize ; ++ i){
            //printf("VGAbuffer %p %p\n",VGAbuffer + i * pitch,buffer + i * Xsize *4);
            //memcpy(VGAbuffer + i * pitch,buffer + i * Xsize,Xsize * 4);
            memcpy_aligned(VGAbuffer + i * pitch,buffer + i * Xsize,Xsize * 4);
        }
        //printf("Screen Updated");
    }

    void Screen::writeLine(uint nb, uint offset, uint size, Color* buf){
        assert(offset + size <= Xsize);
        assert(nb <= Ysize);
        memcpy(VGAbuffer +nb * pitch + 4* offset,buf,size*4);
        memcpy(buffer + nb*Xsize + offset, buf, size*4);
    }

    void Screen::clear(){
        u32 RAMsize = Ysize*Xsize*4;
        memset(buffer,0,RAMsize);
    }

    void Screen::clear(Vec2u offset,Vec2u size){
        for(uint y = offset.y ; y < offset.y + size.y; ++y){
            memset(buffer + Xsize*y + offset.x, 0, size.x*4);
        }
    }

    void Screen::putChar(char c, uint x, uint y, const Font& font, Color fg, Color bg){
        assert(x + 8 < Xsize);
        assert(y + 16 < Ysize);
        for(int i= 0 ; i < 16 ;++i){
            font.writeLine(c,i,(uint*)&buffer[x + (y + i)*Xsize],fg,bg);
        }
    }

    Screen screen;
};
