#include "Screen.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"
#include <string.h>

namespace video {
    Color Color::white{255,255,255,0};
    Color Color::black{0,0,0,0};
    char* const Screen::VGAbuffer = (char*)-0x140000000ll;
    Color* const Screen::buffer= (Color*)-0x120000000ll;
    void Screen::init(GraphicalParam* gp){
        Xsize = gp->Xsize;
        Ysize = gp->Ysize;
        pitch = gp->pitch;
        printf("Pitch :%d\n",pitch);


        // VRAM mapping
        u32 VRAMsize = Ysize*pitch;
        printf("VGAbuffer %p, VRAMsize %d\n",VGAbuffer,VRAMsize);
        paging.createMapping(gp->physptr,VGAbuffer,(int)VRAMsize/0x1000,true);

        // RAM mapping
        u32 RAMsize = Ysize*Xsize*4;
        printf("buffer %p, RAMsize %d\n",buffer,RAMsize);
        for(u32 i =0 ; i < RAMsize / 0x1000 ; ++i){
            paging.createMapping(physmemalloc.alloc(),buffer + i * (0x1000 / 4));
        }
        clear();
        /*for(int i = 0 ; i < 766 * 1024 ; ++i){
            buffer[i] = Color::white;
        }
        send();*/

    }
    void Screen::set(int x, int y,Color c){
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
    void Screen::clear(){
        u32 RAMsize = Ysize*Xsize*4;
        memset(buffer,0,RAMsize);
    }
    void Screen::putChar(char c,int x, int y,const Font& font,Color fg,Color bg){
        for(int i= 0 ; i < 16 ;++i){
            font.writeLine(c,i,(uint*)&buffer[x + (y + i)*Xsize],fg,bg);
        }
    }
    Screen screen;
};
