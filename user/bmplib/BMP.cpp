#include "BMP.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

void BMP::load(std::string file){
    FILE* f = fopen(file.data(),"r");
    if(!f){
        _error = "Can't open file";
        return;
    }
    BMPHeader h;
    int i = fread(&h,sizeof(h),1,f);
    if(i == 0){
        _error = "Can't read header";
        return;
    }
    _height = h.height;
    _width = h.width;
    printf("size : %d * %d",_height,_width);
    if(h.depth == 32) {
        int realwidth = ((h.width*4 +4) /4)*4;
        printf("realwidth %d\n",realwidth);
        printf("size %x\n",h.size);
        _buffer32_2 = (Color32*)malloc(_width*_height *4);
        for(u32 i = 0 ; i < _height ; ++i){
            fseek(f,h.offset + i * realwidth,SEEK_SET);
            int j = fread(_buffer32_2 + i * _width,3,_width,f);
            if(j < _width){
                _error = "Can't read line";
                return;
            }
        }
    } else if(h.depth == 24) {
        int realwidth = ((h.width*3 +3) /4)*4;
        printf("realwidth %d\n",realwidth);
        printf("size %x\n",h.size);
        _buffer24 = (Color24*)malloc(_width*_height *3);
        for(u32 i = 0 ; i < _height ; ++i){
            fseek(f,h.offset + i * realwidth,SEEK_SET);
            int j = fread(_buffer24 + i * _width,3,_width,f);
            if(j < _width){
                _error = "Can't read line";
                return;
            }
        }
    } else {
        assert(false);
    }
}

void BMP::to32(){
    if(_buffer32) return;
    if(_buffer32_2) {
        _buffer32 = (Color32*)malloc(_width*_height *4);
        for(u32 i = 0 ; i < _height ; ++i){
            for(u32 j = 0 ; j < _width ; ++j){
                _buffer32[(_height -1 -i) * _width + j] = _buffer32_2[i * _width + j];
            }
        }
    } else if(_buffer24) {
        _buffer32 = (Color32*)malloc(_width*_height *4);
        for(u32 i = 0 ; i < _height ; ++i){
            for(u32 j = 0 ; j < _width ; ++j){
                _buffer32[(_height -1 -i) * _width + j] = _buffer24[i * _width + j];
            }
        }
    } else {
        _error = "to32 without load";
        return;
    }

}


void BMP::draw(int windowfd){
    to32();

    FILE*f = (FILE*)&windowfd;
    fseek(f,0,SEEK_SET);
    int i = fwrite(_buffer32,4,_height*_width,f);
    printf("drawn %d pixels\n",i);
}
