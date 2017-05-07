#include "GraphWindow.h"
#include <string.h>

namespace video{

    void GraphWindow::send() const{
        uint lim = _size.y + _offset.y;
        for(uint i = _offset.y ; i < lim ; ++i){
            screen.writeLine(i,_offset.x,_size.x,_buffer + i * _size.x);
        }
    }
    void GraphWindow::draw(Vec2u offset,Vec2u size,Color* buffer){
        for(uint i = 0; i < size.y ; ++i){
            memcpy(_buffer + (i + offset.y)*_size.x + offset.x,buffer + i * size.x,size.x);
        }
    }
    void GraphWindow::writeaddr (u64 addr,const void * data, size_t size){
        assert(addr + size < _size.area() *4);
        memcpy((u8*)_buffer + addr,data,size);
    }
    void GraphWindow::readaddr (u64 addr, void * data, size_t size) const{
        assert(addr + size < _size.area() *4);
        memcpy(data,(u8*)_buffer + addr,size);
    }
};
