#include <string.h>
#include "Window.h"

namespace video{
    const Color edge = Color::red;
    uint Window::globWid = 0;
    Window::Window(Vec2u offset, Vec2u size)
        : _offset(offset),_size(size), _wid(globWid++),_active(false),
          hasEdge(true){
    }
    void Window::drawEdge(){
        if(!hasEdge) return;
        for(uint i = 0 ; i < _size.x ; ++i){
            screen.set(_offset.x + i,_offset.y,Color::red);
            screen.set(_offset.x + i,_offset.y + _size.y -1,Color::red);
        }
        for(uint i = 1 ; i < _size.y ; ++i){
            screen.set(_offset.x,_offset.y + i,Color::red);
            screen.set(_offset.x + _size.x -1,_offset.y + i,Color::red);
        }
    }

    bool Window::isInside(const Vec2u& point) {
        return point.x >= _offset.x
            && point.y >= _offset.y
            && point.x <  _offset.x + _size.x
            && point.y <  _offset.y + _size.y;
    }

    Vec2u Window::getOffset() const {
        return _offset;
    }

    void Window::setOffset(const Vec2u& v) {
        _offset = v;
    }

    Vec2u Window::getSize() const {
        return _size;
    }

    void Window::setSize(const Vec2u& v) {
        _size = v;
    }


};

