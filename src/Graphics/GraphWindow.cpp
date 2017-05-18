#include "GraphWindow.h"
#include <string.h>
#include "../log.h"

namespace video{
    GraphWindow::GraphWindow(Vec2u offset, Vec2u size): Window(offset,size) {
        _buffer = new Color[size.area()];
        memset(_buffer,0,4*size.area());
    }

    GraphWindow::~GraphWindow(){
        delete _buffer;
    }

    void GraphWindow::setSize(const Vec2u& v){
        if(v.area() > _size.area()){
            delete _buffer;
            _size = v;
            _buffer = new Color[_size.area()];
        }
        else {
            _size = v;
            memset(_buffer,0,4*_size.area());
        }
    }

    void GraphWindow::send() const{
        //debug("in draw of size %d %d at %d %d", _size.x, _size.y, _offset.x, _offset.y);
        //debug ("Color of 0 0 %d %d", _buffer[0].R,_buffer[0].B);
        
        uint lim = _size.y + _offset.y;
        for(uint i = _offset.y ; i < lim ; ++i){
            //debug("buffer offset %d", i * _size.x);
            screen.writeLine(i,_offset.x,_size.x,_buffer + (i - _offset.y) * _size.x);
        }
        //debug("out draw");
    }
    void GraphWindow::draw(Vec2u offset,Vec2u size,Color* buffer){
        for(uint i = 0; i < size.y ; ++i){
            memcpy(_buffer + (i + offset.y)*_size.x + offset.x,buffer + i * size.x,size.x);
        }
    }
    void GraphWindow::writeaddr (u64 addr,const void * data, size_t size){
        debug("write on GWin at %p of %d",addr,size);
        /*for(int i = 0 ; i < size ; ++i){
            info("%x ",((char*) data)[i]);
            }*/
        //stop;

        assert(addr + size <= _size.area() *4);
        memcpy((u8*)_buffer + addr,data,size);
    }
    void GraphWindow::readaddr (u64 addr, void * data, size_t size) const{
        assert(addr + size <= _size.area() *4);
        memcpy(data,(u8*)_buffer + addr,size);
    }
};
