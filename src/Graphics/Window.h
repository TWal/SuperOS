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
        bool _active;
        Vec2u _offset;
        Vec2u _size;
    public:
        bool hasEdge;
        /// Create window
        Window(Vec2u offset, Vec2u size);
        /// Draw a red line around the window to represent the focus
        void drawEdge();
        /**
           @brief Draw window on screen.

           This call performs a direct copy of the window content to VRAM.
           Thus, it is the bottle neck of the video system.

           If the window is not active, a call to this function has no effect.
        */
        virtual void send() const = 0;
        /// just change internal state (@ref _active) to have an active mode
        void show(){_active = true;}
        /// just change internal state (@ref _active) to have an inactive mode.
        void hide(){_active = false;}
        ///handle an event on this window : return true if the event has been handled.
        virtual bool handleEvent(input::Event e) = 0;

        /// get WID of window.
        uint getWID() const {return _wid;}



    };
}

#endif
