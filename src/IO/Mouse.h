#ifndef MOUSE_H
#define MOUSE_H

#include "../utility.h"
#include "../Deque.h"

struct MouseEvent {
    MouseEvent();
    MouseEvent(u16 x_, u16 y_, u8 pressed_, u8 lastPressed_);
    u16 x;
    u16 y;
    u8 pressed;
    u8 changed;
    inline bool isValid() {
        return pressed != 255 && changed != 255;
    }
    enum Button {
        LEFT=0, RIGHT, MIDDLE
    };
    enum State {
        PRESSED, CHANGED, JUSTPRESSED, JUSTRELEASED
    };
    inline bool get(Button b, State s) {
        u8 v;
        switch(s) {
            case PRESSED:
                v = pressed;
                break;
            case CHANGED:
                v = changed;
                break;
            case JUSTPRESSED:
                v = pressed & changed;
                break;
            case JUSTRELEASED:
                v = (~pressed) & changed;
                break;
        }
        return (v & (1 << b)) != 0;
    }
};

static_assert(sizeof(MouseEvent) == 6, "size of MouseEvent");

class Mouse {
    public:
        Mouse();
        void init();
        void handleByte(u8 b);
        MouseEvent poll();
        MouseEvent pollLast();
        void draw();
    private:
        void _wait(bool type);
        void _write(u8 data);
        u8 _read();
        i32 _totalX;
        i32 _totalY;
        u8 _byte[3];
        u8 _cycle;
        u8 _lastPressed;
        Deque<MouseEvent, 1024> _deque;
};

extern Mouse mouse;

#endif

