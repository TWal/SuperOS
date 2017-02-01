#include "Keyboard.h"


const Keymap azertyKeymap = {
    {
        -1, 0x1b,
        '&', 'e', '"', '\'', '(', '-', 'e', '_', 'c', 'a', ')', '=',
        '\b', '\t',
        'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$',
        '\n', -1,
        'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u',
        '2',
        -1,
        '*',
        'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!',
        -1, -1, ' ', -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }, {
        -1, 0x1b,
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'o', '+',
        '\b', '\t',
        'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '^', '$',
        '\n', -1,
        'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'U',
        '~',
        -1,
        'u',
        'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 'S'
        -1, -1, ' ', -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }
};

const Keymap dvorakKeymap = {
    {
        -1, 0x1b,
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '[', ']',
        '\b', '\t',
        '\'', ',', '.', 'p', 'y', 'f', 'g', 'c', 'r', 'l', '/', '=',
        '\n', -1,
        'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's', '-',
        '`',
        -1,
        '\\',
        ';', 'q', 'j', 'k', 'x', 'b', 'm', 'w', 'v', 'z',
        -1, -1, ' ', -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }, {
        -1, 0x1b,
        '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '{', '}',
        '\b', '\t',
        '"', '<', '>', 'P', 'Y', 'F', 'G', 'C', 'R', 'L', '?', '+',
        '\n', -1,
        'A', 'O', 'E', 'U', 'I', 'D', 'H', 'T', 'N', 'S', '_',
        '~',
        -1,
        '|',
        ':', 'Q', 'J', 'K', 'X', 'B', 'M', 'W', 'V', 'Z',
        -1, -1, ' ', -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }
};

Keyboard::Keyboard() : _deque(), _flags(0), _lastIsE0(false) { }

void Keyboard::handleScanCode(uchar sc) {
    _deque.push_back(sc);
}

Keycode Keyboard::poll() {
    //TODO: handle e0
    while(true) {
        if(_deque.empty()) {
            asm volatile("hlt");
        }
        uchar sc = _deque.front();
        _deque.pop_front();

        const uchar RELEASE = 0x80;
        const uchar CTRL_SC = 0x1d;
        const uchar SHIFT_SC = 0x2a;
        const uchar ALT_SC = 0x38;
        const uchar CAPSLOCK_SC = 0x3a;

        bool release = (sc & RELEASE) != 0;
        char bit = -1;
        uchar rsc = sc & ~RELEASE;
        if(rsc == CTRL_SC) {
            bit = LCTRL;
        } else if(rsc == SHIFT_SC) {
            bit = LSHIFT;
        } else if(rsc == ALT_SC) {
            bit = LALT;
        } else if(rsc == CAPSLOCK_SC) {
            bit = CAPSLOCK;
        }
        if(bit >= 0) {
            if(release) {
                _flags &= ~(1<<bit);
            } else {
                _flags |= (1<<bit);
            }
        }

        char symbol;
        if((sc & ~RELEASE) > 0x45) {
            symbol = -1;
        } else {
            if(_flags & ((1<<LSHIFT) | (1<<RSHIFT) | (1<<CAPSLOCK))) {
                symbol = _keymap->shift[sc & ~RELEASE];
            } else {
                symbol = _keymap->noShift[sc & ~RELEASE];
            }
        }

        Keycode res;
        res.flags = _flags;
        res.symbol = symbol;
        res.scanCode = sc;
        res.isRelease = release;
        return res;
    }
}
void Keyboard::setKeymap(const Keymap* km) {
    _keymap = km;
}
