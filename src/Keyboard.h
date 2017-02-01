#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utility.h"
#include "Deque.h"

struct Keycode {
    uint flags;
    uchar scanCode;
    char symbol;
    bool isRelease;
};

struct Keymap {
    char noShift[0x45];
    char shift[0x45];
};

extern const Keymap azertyKeymap;
extern const Keymap dvorakKeymap;

class Keyboard {
    public:
        Keyboard();
        void handleScanCode(uchar sc);
        Keycode poll();
        void setKeymap(const Keymap* km);
        enum Flags {
            LSHIFT = 0,
            RSHIFT,
            LCTRL,
            RCTRL,
            LALT,
            RALT,
            CAPSLOCK
        };
    private:
        Deque<uchar, 16> _deque;
        uint _flags;
        bool _lastIsE0;
        const Keymap* _keymap;
};

#endif

