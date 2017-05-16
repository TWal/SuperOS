#include "TextWindow.h"
#include <string.h>
#include <errno.h>

using namespace input;

namespace video{
    void TextWindow::newLine(){
        _curs.x = 0;
        if(_curs.y + 1 == _lineBuffer.size()){
            if(_lineBuffer.size() == LineNum){
                _lineBuffer.pop_front();
            }
            else{
                ++_curs.y;
            }
            _lineBuffer.push_back(Line());
        }
        else ++_curs.y;
    }

    void TextWindow::addPrintableChar(char c){
        //fprintf(stderr,"pos %d %d %d %d %d\n",_curs.x, _curs.y,_cursFormat.fg.R,
        //_cursFormat.fg.G,_cursFormat.fg.B);
        _lineBuffer.at(_curs.y).write(_curs.x,FormatedChar(c,_cursFormat));
        ++_curs.x;
    }

    size_t TextWindow::write(const void* buf, size_t count){
        //fprintf(stderr,"write TextWindow\n");
        const char* buf2 = reinterpret_cast<const char*>(buf);
        const size_t limit = 1024;
        if(count > limit) count = limit;
        for(size_t i = 0 ; i < count ; ++i){
            putChar(buf2[i]);
        }
        return count;
    }

    void TextWindow::error(){
        _state = NORMAL;
        putChar('~');
        putChar('~');
    }

    void TextWindow::putChar(char c){
        //fprintf(stderr,"putChar('%c'); with %d in win %d\n",c,_state,getWID());
        switch(_state){
            case WAITINGOB:
                if(c != '['){
                    error();
                    putChar(c);
                }
                else {
                    _stack.clear();
                    _stack.push_back(0);
                    _state = ESCAPE;
                }
                return;


            case ESCAPE:
                if(c < 32 or c > 126) {
                    error();
                    putChar(c);
                    return;
                }

                if(c >= '0' and c <= '9'){
                    _stack.back() *=10;
                    _stack.back() += (c - '0');
                    return;
                }

                switch(c){
                    case ';':
                        _stack.push_back(0);
                        return;
                    case 'm':
                        SGR();
                        _state = NORMAL;
                        return;
                    default:
                        error();
                        putChar(c);
                        return;
                }


            case NORMAL:
                if(c == 27){
                    _state = WAITINGOB;
                    return;
                }

                uint val;
                switch(c){
                    case '\n':
                        //fprintf(stderr," new line \n");
                        newLine();
                        return;

                    case '\t':
                        // A clean way to do it.
                        val = _curs.x%4;
                        for(uint i = 0 ; i < 4-val ; ++i){
                            addPrintableChar(' ');
                        }
                        return;

                    case '\b':
                        if(_curs.x == 0){
                            if(_curs.y > 0) --_curs.y;
                            _curs.x = _width -1;
                        }
                        else --_curs.x;
                        return;

                    case '\r':
                        _curs.x = 0;
                        return;

                    case '\f':
                        val = _curs.x;
                        newLine();
                        _curs.x = val;
                        return;

                    default:
                        if(c < 32 and c >= EOF){
                            error();
                            return;
                        }
                        addPrintableChar(c);
                        return;
                }
        }
    }

    /**
       @brief Handles SGR Commands

       @todo Add Blinking.
     */

    void TextWindow::SGR(){
        //fprintf(stderr,"SGR %d\n",_stack.size());
        if(_stack.size() == 0){
            _cursFormat = Format();
            return;
        }

        uint command = _stack[0];
        if(command == 0){
            _cursFormat = Format();
        }
        else if(command == 1){
            _bright = true;
        }
        else if(command == 2){
            _bright = false;
        }
        else if(command == 4){
            _cursFormat.underline = true;
        }
        else if(command == 7){
            std::swap(_cursFormat.fg,_cursFormat.bg);
        }
        else if(command == 24){
            _cursFormat.underline = false;
        }

        else if(command >=30 and command <= 37){
            _cursFormat.fg = getByNum(command -30);
        }
        else if(command == 38){
            if(_stack.size() < 5){
                error();
                return;
            }
            if(_stack[1] != 2){
                error();
                return;
            }
            _cursFormat.fg.R = _stack[3];
            _cursFormat.fg.G = _stack[4];
            _cursFormat.fg.B = _stack[5];
        }
        else if(command == 39){
            _cursFormat.fg = Color24::lwhite;
        }

        else if(command >=40 and command <= 47){
            _cursFormat.bg = getByNum(command -40);
        }
        else if(command == 48){
            if(_stack.size() < 5){
                error();
                return;
            }
            if(_stack[1] != 2){
                error();
                return;
            }
            _cursFormat.bg.R = _stack[3];
            _cursFormat.bg.G = _stack[4];
            _cursFormat.bg.B = _stack[5];
        }
        else if(command == 49){
            _cursFormat.bg = Color24::black;
        }
    }

    void TextWindow::send() const {
        screen.clear(_offset, _size);
        //avoid infinite loop
        if(_width == 0) return;

        uint height = 0;
        auto it = _lineBuffer.end();
        while(it != _lineBuffer.begin()){
            --it;
            height += (it->data.size() + _width - 1)/_width;
            if(height >= _height) {
                height = _height;
                break;
            }
        }

        it = _lineBuffer.end();
        while(it != _lineBuffer.begin()){
            --it;
            uint nbLine = (it->data.size() + _width - 1)/_width;
            for(uint i_ = 0; i_ < nbLine; ++i_) {
                if(height == 0) return;
                uint i = nbLine - 1 - i_;
                uint startx = i*_width;
                for(uint x = 0; x < min((uint)_width, (uint)(it->data.size()-startx)); ++x) {
                    drawFMC({x, height-1},  it->data[startx + x]);
                }
                height -= 1;
            }
        }
    }

    void TextWindow::drawFMC(Vec2u pos, FormatedChar fmc) const{
        screen.putChar(fmc.c,_offset.x + pos.x*8+1,_offset.y + pos.y*16 + 1,*_font,fmc.fg,fmc.bg);
        if(fmc.underline){
            for(uint k = 0 ; k < 8 ; ++k){
                screen.set(_offset.x + pos.x*8+1+k, _offset.y + pos.y*16+16, fmc.fg);
            }
        }
    }

    void TextWindow::handleEvent(input::Event e){
        if(e.type == Event::KEYBOARD){
            if(e.kcode.symbol and !e.kcode.scanCode.release and allowInput){
                char sym = e.kcode.symbol;
                if(sym == '\b' ){
                    if(!_keyboardBuffer.empty()){
                        _keyboardBuffer.pop_back();
                        putChar('\b');
                        putChar(' ');
                        putChar('\b');
                    }
                }
                else if(sym == '\n'){
                    for(auto c : _keyboardBuffer){
                        _inputBuffer.push_back(c);
                    }
                    _keyboardBuffer.clear();
                    _inputBuffer.push_back('\n');
                    putChar('\n');
                }
                else{
                    _keyboardBuffer.push_back(sym);
                    putChar(sym);
                }
            }
        }
    }

    size_t TextWindow::read(void* buf, size_t count){
        char* buf2 = reinterpret_cast<char*>(buf);
        if(count > 1000) count = 1000;
        size_t reallyRead =0;
        while(!_inputBuffer.empty() and reallyRead < count){
            *buf2 = _inputBuffer.front();
            _inputBuffer.pop_front();
            ++reallyRead;
            ++buf2;
        }
        return reallyRead;
    }

    void TextWindow::setSize(const Vec2u& v) {
        Window::setSize(v);
        _height = (v.y - 2) / 16;
        if(v.x < 2) {
            _width = 0;
        } else {
            _width = (v.x - 2) / 8;
        }
    }
};
