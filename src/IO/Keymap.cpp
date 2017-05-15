#include "Keyboard.h"

namespace input{

    const Keymap qwertyKeymap = {
        { // main
            0, 0,  // NULL + ESC
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  // first line
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', // second line
            '\n', // main Enter
            0, // Lctrl
            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', // third line
            '`', // top left under ESC
            0, //lShift
            '\\', // Somewhere on the right
            'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', // fourth line
            0, // rshift
            '*', // KeyPad *
            0, // LAlt : 0x38
            ' ', // Space
            0, // CapsLock
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F1 to F10
            0, // NumLock
            0, // ScrollLock
            '\r', 0, 0, '-', // first keypad line
            0, 0, 0, '+', // second keypad line
            0, 0, 0, // third keypad line
            0, 0, // last keypad line
            0, 127, // nothing
            0, // key 0x56 (azerty key for < and >)
        },
        { // Shift
            0, 0,  // NULL + ESC
            '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  // first line
            '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', // second line
            '\n', // main Enter
            0, // Lctrl
            'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', // third line
            '~', // top left under ESC
            0, //lShift
            '|', // Somewhere on the right
            'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', // fourth line
            0, // rshift
            '*', // KeyPad *
        },
        0, // shift key 56
        { // altGr first line
            0,0,0,0,0,0,0,0,0,0,0,0,0,0
        },
        { // keypad on numlock
            '7', '8', '9', '-', // first keypad line
            '4', '5', '6', '+', // second keypad line
            '1', '2', '3', // third keypad line
            '0', '.', // last keypad line
        }
    };

    const Keymap azertyKeymap = {
        { // main
            0, 0,  // NULL + ESC
            '&', char(0xe9), '"', '\'', '(', '-', char(0xe8), '_',
            char(0xe7), char(0xe0), ')', '=', '\b',  // first line
            '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', // second line
            '\n', // main Enter
            0, // Lctrl
            'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', char(0xf9), // third line
            char(0xb2), // top left under ESC(² in ISO0 here)
            0, //lShift
            '*', // Somewhere on the right
            'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', // fourth line
            0, // rshift
            '*', // KeyPad *
            0, // LAlt : 0x38
            ' ', // Space
            0, // CapsLock
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F1 to F10
            0, // NumLock
            0, // ScrollLock
            '\r', 0, 0, '-', // first keypad line
            0, 0, 0, '+', // second keypad line
            0, 0, 0, // third keypad line
            0, 0, // last keypad line
            0, 127, // nothing
            '<', // key 0x56 (azerty key for < and >)
        },
        { // Shift
            0, 0,  // NULL + ESC 
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', char(0xb0), '+', '\b',  // first line
            '\t', 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '"', char(0xA3), // second line
            '\n', // main Enter
            0, // Lctrl
            'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', // third line
            0, // top left under ESC
            0, //lShift
            char(0xb5), // Somewhere on the right
            'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', char(0xA7), // fourth line
            0, // rshift
            '*', // KeyPad *
        },
        0, // shift key 56
        { // altGr first line
            0, 0, // NULL + ESC
            0, '~', '#', '{', '[', '|', '`', '\\', '^', '@', ']', '}'
        },
        { // keypad on numlock
            '7', '8', '9', '-', // first keypad line
            '4', '5', '6', '+', // second keypad line
            '1', '2', '3', // third keypad line
            '0', '.', // last keypad line
        }
    };

    const Keymap dvorakKeymap = {
        { // main
            0, 0,  // NULL + ESC
            '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '[', ']', '\b',  // first line
            '\t', '\'', ',', '.', 'p', 'y', 'f', 'g', 'c', 'r', 'l', '/', '=', // second line
            '\n', // main Enter
            0, // Lctrl
            'a', 'o', 'e', 'u', 'i', 'd', 'h', 't', 'n', 's', '-', // third line
            '`', // top left under ESC
            0, //lShift
            '\\', // Somewhere on the right
            ';', 'q', 'j', 'k', 'x', 'b', 'm', 'w', 'v', 'z', // fourth line
            0, // rshift
            '*', // KeyPad *
            0, // LAlt : 0x38
            ' ', // Space
            0, // CapsLock
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F1 to F10
            0, // NumLock
            0, // ScrollLock
            '\r', 0, 0, '-', // first keypad line
            0, 0, 0, '+', // second keypad line
            0, 0, 0, // third keypad line
            0, 0, // last keypad line
            0, 127, // nothing
            0, // key 0x56 (azerty key for < and >)
        },
        { // Shift
            0, 0,  // NULL + ESC
            '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '{', '}', '\b',  // first line
            '\t', '"', '<', '>', 'P', 'Y', 'F', 'G', 'C', 'R', 'L', '?', '+',  // second line
            '\n', // main Enter
            0, // Lctrl
            'A', 'O', 'E', 'U', 'I', 'D', 'H', 'T', 'N', 'S', '_', // third line
            '~', // top left under ESC
            0, //lShift
            '|', // Somewhere on the right
            ':', 'Q', 'J', 'K', 'X', 'B', 'M', 'W', 'V', 'Z', // fourth line
            0, // rshift
            '*', // KeyPad *
        },
        0, // shift key 56
        { // altGr first line
            0,0,0,0,0,0,0,0,0,0,0,0,0,0
        },
        { // keypad on numlock
            '7', '8', '9', '-', // first keypad line
            '4', '5', '6', '+', // second keypad line
            '1', '2', '3', // third keypad line
            '0', '.', // last keypad line
        }
    };

    const Keymap* keymaps[NB_KEYMAPS] = {&azertyKeymap, &qwertyKeymap, &dvorakKeymap};

}
