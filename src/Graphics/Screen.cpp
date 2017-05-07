#include "Screen.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"
#include <string.h>
#include "../log.h"

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

    }
    void Screen::set(uint x, uint y, Color c){
        assert(x < Xsize);
        assert(y < Ysize);
        //printf("Set %d,%d\n",x,y);
        buffer[x + y * Xsize] = c;
    }
    void Screen::send(){
        //printf("Screen Update");
        for(int i = 0 ; i < Ysize ; ++ i){
            //printf("VGAbuffer %p %p\n",VGAbuffer + i * pitch,buffer + i * Xsize *4);
            memcpy(VGAbuffer + i * pitch,buffer + i * Xsize,Xsize * 4);
        }
        //printf("Screen Updated");
    }
    void Screen::writeLine(uint nb, uint offset, uint size, Color* buffer){
        assert(offset + size <= Xsize);
        assert(nb <= Ysize);
        memcpy(VGAbuffer +nb * pitch + 4* offset,buffer,size*4);
    }
    void Screen::clear(){
        u32 RAMsize = Ysize*Xsize*4;
        memset(buffer,0,RAMsize);
    }
    void Screen::clear(Vec2u offset,Vec2u size){
        for(uint y = offset.y ; y < offset.y + size.y; ++y){
            memset(buffer + offset.x,0,size.x);
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
