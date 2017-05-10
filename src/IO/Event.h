#ifndef EVENT_H
#define EVENT_H

#include "Keyboard.h"
#include "Mouse.h"

namespace input{
    struct Event{
        enum Type{KEYBOARD,MOUSE,WINDOW};
        union{
            Keyboard::KeyCode kcode;
            MouseEvent mousec;
            int windowc;
        };
        Type type;
        Event(Keyboard::KeyCode keyCode) : kcode(keyCode), type(KEYBOARD){}
        Event(MouseEvent mouseevent) : mousec(mouseevent), type(MOUSE){}
    };

    /*class EventHandler{
        virtual bool handleEvent(Event e) const = 0;
        };*/

}

#endif
