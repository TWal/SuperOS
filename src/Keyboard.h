#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utility.h"
#include "Deque.h"

struct Keycode {
    u32 flags;
    u8 scanCode;
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
        uchar pollSC();
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
        u32 _flags;
        bool _lastIsE0;
        const Keymap* _keymap;
};

#endif

