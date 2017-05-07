#ifndef EVENT_H
#define EVENT_H

#include"Keyboard.h"

namespace input{
    struct Event{
        enum Type{KEYBOARD,MOUSE,WINDOW};
        union{
            Keyboard::KeyCode kcode;
            int mousec;
            int windowc;
        };
        Type type;
        Event(Keyboard::KeyCode keyCode) : kcode(keyCode),type(KEYBOARD){}
    };

    /*class EventHandler{
        virtual bool handleEvent(Event e) const = 0;
        };*/

}

#endif
