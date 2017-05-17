#ifndef EVENT_H
#define EVENT_H

#include "Keyboard.h"
#include "Mouse.h"
#include "../Graphics/GraphEvent.h"

namespace input{
    struct Event{
        enum Type : char {KEYBOARD,MOUSE,WINDOW,INVALID};
        Type type;
        union{
            Keyboard::KeyCode kcode;
            MouseEvent mousec;
            video::GraphEvent gevent;
        }__attribute__((packed));
        Event(Keyboard::KeyCode keyCode) : type(KEYBOARD), kcode(keyCode){}
        Event(MouseEvent mouseevent) : type(MOUSE), mousec(mouseevent) {}
        Event(video::GraphEvent graphEvent) : type(WINDOW), gevent(graphEvent) {}
        Event() : type(INVALID){}
        operator u64(){return *reinterpret_cast<u64*>(this);}
    }__attribute__((packed));

    static_assert(sizeof(Event) == 7, "Event too big");
}

#endif
