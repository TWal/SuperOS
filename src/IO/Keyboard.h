#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../utility.h"
#include "../Deque.h"


/**
   @brief Contains all input code, keyboard mouse and event system
 */
namespace input{

    /// A keymap to convert scan codes to ASCII
    struct Keymap {
        char main[0x57]; // no modifiers
        char shift[0x38];
        char shift56;
        char altGr[0xe]; // only first line
        char pad[0x54 - 0x47]; // Index with Ox47 offset;
    };

    extern const Keymap qwertyKeymap;
    extern const Keymap azertyKeymap;
    extern const Keymap dvorakKeymap;

    const uint NB_KEYMAPS = 3;
    extern const Keymap* keymaps[NB_KEYMAPS];

    class Keyboard {
    public:
        struct State{
            State(){*reinterpret_cast<u8*>(this) = 0;}
            bool lShift : 1;
            bool rShift : 1;
            bool lCtrl  : 1;
            bool rCtrl  : 1;
            bool lAlt   : 1;
            bool rAlt   : 1; // Alt Gr
            bool lWin   : 1;
            bool rWin   : 1;
            bool capsLock : 1;
            bool numLock : 1;
        }__attribute__((packed));
        static_assert(sizeof(State) == 2,"Wrong Size for Keyboard::State");

        enum Code{
            ERROR, ESC, FL1, FL2, FL3, FL4, FL5, FL6, FL7, FL8, FL9, FL10, FL11, FL12,
            BACKSPACE, TAB,
            SL1, SL2, SL3, SL4, SL5, SL6, SL7, SL8, SL9, SL10, SL11, SL12,
            ENTER, CTRL,
            TL1, TL2, TL3, TL4, TL5, TL6, TL7, TL8, TL9, TL10, TL11,
            TOPLEFT, LSHIFT, SOMERIGHT,
            FOL1, FOL2, FOL3, FOL4, FOL5, FOL6, FOL7, FOL8, FOL9, FOL10,
            RSHIFT, KSTAR, ALT, SPACE, CAPSLOCK,
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
            NUMLOCK, SCROLL_LOCK,
            K7, K8, K9, KMINUS,
            K4, K5, K6, KPLUS,
            K1, K2, K3,
            K0, KDEL, T56 = 0x56, LWIN = 0x5b, RWIN = 0x5c
        };

        struct ScanCode{
            inline ScanCode(uchar c){*reinterpret_cast<uchar*>(this) = c;}
            u8 code : 7;
            bool release :1;
        }__attribute__((packed));
        static_assert(sizeof(ScanCode) == 1,"Wrong Size for ScanCode");

        struct EScanCode : public ScanCode{
            inline EScanCode() : ScanCode(-1),extended(false),valid(false){};
            inline EScanCode(uchar c) : ScanCode(c),extended(false),valid(true){};
            bool extended : 1;
            bool valid : 1;
        }__attribute__((packed));
        static_assert(sizeof(EScanCode) == 2,"Wrong Size for EScanCode");

        struct KeyCode {
            State state;
            char symbol; // ASCII (0 if not printable)
            EScanCode scanCode;
        };
        static_assert(sizeof(KeyCode) == 5,"Wrong Size for KeyCode");
        Keyboard();
        void handleScanCode(uchar sc);
        KeyCode poll();
        EScanCode pollSC();
        void setKeymap (const Keymap* km);
        void setLeds(u8 f);
    private:
        Deque<uchar, 1024> _deque;
        State _state;
        const Keymap* _keymap;
    };


    extern Keyboard kbd;

}

#endif

