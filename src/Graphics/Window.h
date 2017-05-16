#ifndef WINDOW_H
#define WINDOW_H

#include "Vec2.h"
#include "../Bytes.h"
#include "Screen.h"
#include "../IO/Event.h"

namespace video{

    /**
       @brief Represent a GUI window.

       The window has a given size, at a given offset on the screen.

       The Bytes interface is that of table of _size.area() element of type Color.
       The buffer is structured line by line.

     */

    class Window{
        uint _wid; ///< window ID
        static uint globWid;
    protected:
        Vec2u _offset;
        Vec2u _size;
    public:
        /// Create window
        Window(Vec2u offset, Vec2u size);
        /// Delete window
        virtual ~Window(){}
        /// Draw a line around the window to represent the focus
        void drawEdge(const Color& color);

        /**
           @brief Draw window on screen.

           This call performs a direct copy of the window content to VRAM.
           Thus, it is the bottle neck of the video system.

           If the window is not active, a call to this function has no effect.
        */
        virtual void send() const = 0;

        ///handle an event on this window
        virtual void handleEvent(input::Event e) = 0;
        /// get WID of window.
        uint getWID() const {return _wid;}
        /// check if a point is in the window
        bool isInside(const Vec2u& point);

        Vec2u getOffset() const;
        void setOffset(const Vec2u& v);
        Vec2u getSize() const;
        virtual void setSize(const Vec2u& v);
    };
}

#endif
