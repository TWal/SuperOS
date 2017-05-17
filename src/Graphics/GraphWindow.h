#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <deque>
#include "Window.h"
#include "../IO/Event.h"

namespace video{
    class GraphWindow : public Window, public Bytes{
        Color* _buffer; //heap memory for video Buffer
        std::deque<input::Event> _events;
    public :
        GraphWindow(Vec2u offset, Vec2u size);
        ~GraphWindow();
        /// Draw a buffer at offset. buffer must be of size size.area();
        void draw(Vec2u offset,Vec2u size,Color* buffer);
        /// Send window to screen
        void send() const;
        /// Handle an event. @todo put it on the event FIFO
        void handleEvent(input::Event e){
            _events.push_back(e);
        }
        /**
           @brief Change the size of the window.

           It also clears the buffer (window becomes black).

           it has nothing to do with @ref getSize from bytes Interface.
        */
        void setSize(const Vec2u& v);

        /**
           @brief Returns and unhandled event (for user mode normally).
         */
        input::Event getEvent(){
            if(_events.empty()) return input::Event();
            input::Event e = _events.front();
            _events.pop_front();
            return e;
        }

        // Bytes interface

        /// Write `data` of size `size` at the address `addr`
        void writeaddr (u64 addr,const void * data, size_t size);
        /// Read `data` of size `size` at the address `addr`
        void readaddr (u64 addr, void * data, size_t size) const;
        /// Get the size of this sequence. 0 means "unknown"
        /// @todo Name conflict between Bytes::getSize and Window::getSize
        size_t getSize() const{return 4 * _size.area();}
    };
};


#endif
