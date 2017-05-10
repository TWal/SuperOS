#include "Keyboard.h"
#include "../log.h"

namespace input{
    Keyboard::Keyboard() : _deque(){ }

    void Keyboard::handleScanCode(uchar sc) {
        _deque.push_back(sc);
    }

//Code qui peut être modifié par une interruption : __attribute__((optimize("O0")))
    Keyboard::EScanCode __attribute__((optimize("O0"))) Keyboard::pollSC(){
        /*while(_deque.empty()) {
          asm volatile("hlt");
          }*/
        if(_deque.empty()) return EScanCode();

        uchar sc = _deque.front();
        if(sc == 0xe0){
            if(_deque.size() <= 1) return EScanCode();
            EScanCode e (_deque[1]);
            debug(Kbd,"Polled scan code extended : %x",_deque[1]);
            e.extended = true;
            _deque.pop_front();
            _deque.pop_front();
            return e;
        }
        debug(Kbd,"Polled scan code : %x",sc);
        _deque.pop_front();
        return sc;
    }


    Keyboard::KeyCode Keyboard::poll() {
        while(true) {

            EScanCode sc = pollSC();
            if(!sc.valid){ // EOF
                return {_state,EOF,sc};
            }
            /*const uchar CTRL = 0x1d;
            const uchar LSHIFT = 0x2a;
            const uchar RSHIFT = 0x36;
            const uchar ALT = 0x38;
            const uchar CAPSLOCK = 0x3a;
            const uchar NUMLOCK = 0x45;*/

            switch(sc.code){
                case CTRL:
                    if(sc.extended) _state.rCtrl = !sc.release;
                    else            _state.lCtrl = !sc.release;
                    break;
                case LSHIFT:
                    _state.lShift = !sc.release;
                    break;
                case RSHIFT:
                    _state.rShift = !sc.release;
                    break;
                case ALT:
                    if(sc.extended) _state.rAlt = !sc.release;
                    else            _state.lAlt = !sc.release;
                    break;
                case CAPSLOCK:
                    if(!sc.release) _state.capsLock = !_state.capsLock;
                    setLeds((_state.capsLock << 2) + (_state.numLock << 1));
                    break;
                case NUMLOCK:
                    if(!sc.release) _state.numLock = !_state.numLock;
                    setLeds((_state.capsLock << 2) + (_state.numLock << 1));
                    break;
                case LWIN:
                    if(sc.extended) _state.lWin = !sc.release;
                    break;
                case RWIN:
                    if(sc.extended) _state.rWin = !sc.release;
                    break;
            };

            // to ASCII
            char symbol = 0;
            if(sc.code >= 0x57);
            else if(sc.extended and sc.code == 0x35) symbol = '/';
            else if(sc.extended);

            else if(_state.lShift or _state.rShift or _state.capsLock){ // shift
                if(sc.code == 0x56){
                    symbol = _keymap->shift56;
                }
                else if(sc.code >= 0x47 and sc.code < 0x54){
                    if(_state.numLock and !sc.extended) symbol = _keymap->main[sc.code -0x47];
                    else symbol = _keymap->pad[sc.code - 0x47];
                }
                else if(sc.code < 0x38) symbol = _keymap->shift[sc.code];
            }

            else if(_state.rAlt){ // alt gr
                if(sc.code < 0xe) symbol = _keymap->altGr[sc.code];
            }

            else{ // no shift, no alt gr
                if(sc.code >= 0x47 and sc.code < 0x54 and _state.numLock and !sc.extended){
                    symbol = _keymap->pad[sc.code - 0x47];
                }
                else symbol = _keymap->main[sc.code];
            }
            if(symbol) debug(Kbd,"Polled char %c",symbol);
            else debug(Kbd,"Polled non printable char");

            return {_state,symbol,sc};
        }
    }
    void Keyboard::setKeymap(const Keymap* km) {
        _keymap = km;
    }
    void Keyboard::setLeds(u8 f){
        outb(0x60,0xED);
        WAIT(10000);
        outb(0x60,f);
        WAIT(10000);
        debug(Kbd,"Led output %x",inb(0x60));
    }

    Keyboard kbd;
}
