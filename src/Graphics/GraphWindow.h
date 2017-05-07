#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include "Window.h"

namespace video{
    class GraphWindow : public Window, public Bytes{
        Color* _buffer; //heap memory for video Buffer
        /// draw a buffer at offset. buffer must be of size size.area();
        void draw(Vec2u offset,Vec2u size,Color* buffer);
        void send() const;
        
        // Bytes interface

        /// Write `data` of size `size` at the address `addr`
        void writeaddr (u64 addr,const void * data, size_t size);
        /// Read `data` of size `size` at the address `addr`
        void readaddr (u64 addr, void * data, size_t size) const;
        /// Get the size of this sequence. 0 means "unknown"
        size_t getSize() const{return 4 * _size.area();}
        
    };
};


#endif
