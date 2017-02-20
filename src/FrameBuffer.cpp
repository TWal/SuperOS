#include "FrameBuffer.h"
#include "globals.h"

static char* const FB = (char*) (0xB8000 + THREEGB);

inline static char getColor(char fg, char bg) {
    return (fg & 0xF) | ((bg & 0xF) << 4);
}

FrameBuffer::FrameBuffer() {
    clear();
}

void FrameBuffer::clear(char fg, char bg) {
    _fg = fg;
    _bg = bg;
    _cursCol = 0;
    _cursRow = 0;
    _margLeft = 0;
    _margRight = 80;
    char color = getColor(_fg, _bg);
    for(int i = 0 ; i < 80 * 25 ; ++i){
        FB[2*i] = 0;
        FB[2*i+1] = color;
    }
}

void FrameBuffer::moveCursor(int col, int row, bool updateCurs) {
    _cursCol = col;
    _cursRow = row;
    if(updateCurs) updateCursor();
}

void FrameBuffer::updateCursor() {
    static const ushort CURSOR_COMMAND_PORT = 0x3D4;
    static const ushort CURSOR_DATA_PORT = 0x3D5;
    static const uchar CURSOR_HIGH_BYTE = 0xE;
    static const uchar CURSOR_LOW_BYTE = 0xF;

    int pos = 80*_cursRow + _cursCol;
    outb(CURSOR_COMMAND_PORT, CURSOR_HIGH_BYTE);
    outb(CURSOR_DATA_PORT, (pos>>8) & 0xFF);
    outb(CURSOR_COMMAND_PORT, CURSOR_LOW_BYTE);
    outb(CURSOR_DATA_PORT, pos & 0xFF);
}

void FrameBuffer::setMargin(int left, int right) {
    _margLeft = left;
    _margRight = right;
}

void FrameBuffer::writeChar(char c, int col, int row, char fg, char bg) {
    int i = 80*row + col;
    FB[2*i] = c;
    FB[2*i+1] = getColor(fg, bg);
}

void FrameBuffer::scroll(uint n, bool updateCurs) {
    char color = getColor(_fg, _bg);
    for(int row = 0; row < 25; ++row) {
        for(int col = 0; col < 80; ++col) {
            int posTo = 80*row+col;
            if(row+n < 25) {
                int posFrom = 80*(row+n)+col;
                FB[2*posTo] = FB[2*posFrom];
                FB[2*posTo+1] = FB[2*posFrom+1];
            } else {
                FB[2*posTo] = 0;
                FB[2*posTo+1] = color;
            }
        }
    }

    _cursRow = max(_cursRow - (int)n, 0);
    if(updateCurs) updateCursor();
}

void FrameBuffer::putc(char c, bool updateCurs) {
    switch(c) {
        case '\n':
            _cursCol = _margLeft;
            _cursRow++;
            break;
        case '\r':
            _cursCol = _margLeft;
            break;
        case '\f':
            _cursRow++;
            break;
        case '\t':
            _cursCol += 4-(_cursCol%4);
            break;
        case '\b':
            if(_cursCol <= _margLeft) {
                if(_cursRow != 0) {
                    _cursCol = _margRight-1;
                    --_cursRow;
                }
            } else {
                --_cursCol;
            }
            break;
        default:
            writeChar(c, _cursCol, _cursRow, _fg, _bg);
            _cursCol++;
            break;
    }
    if(_cursCol >= _margRight) {
        _cursCol -= _margRight - _margLeft;
        _cursRow += 1;
    }
    if(_cursRow >= 25) {
        scroll(1, false);
    }
    if(updateCurs) updateCursor();
}

void FrameBuffer::puts(const char* s, bool updateCurs) {
    for(; *s; ++s) {
        putc(*s, false);
    }
    if(updateCurs) updateCursor();
}


void FrameBuffer::printDigit(int d, bool updateCurs) {
    if(d <= 9) {
        putc('0' + d, false);
    } else {
        putc('a' + (d-10), false);
    }
    if(updateCurs) updateCursor();
}

void FrameBuffer::printInt(int n, uint base, uint padding, bool updateCurs) {
    if(n < 0) {
        putc('-', false);
        n = -n;
    }
    printUInt(n,base,padding,updateCurs);
}

void FrameBuffer::printUInt(uint n, uint base, uint padding, bool updateCurs){
    uint i = 1;
    while(n/i >= base) {
        i *= base;
        if(padding > 0) {
            padding -= 1;
        }
    }

    for(uint j = 1; j < padding; ++j) {
        putc('0', false);
    }

    while(n > 0) {
        printDigit(n/i, false);
        n %= i;
        i /= base;
    }
    while(i >= 1) {
        putc('0', false);
        i /= base;
    }
    if(updateCurs) updateCursor();
}

void FrameBuffer::vprintf(const char* s, va_list ap) {
    while(*s) {
        if(*s != '%') {
            putc(*s, false);
            ++s;
            continue;
        }
        ++s;
        int padding = 0;
        while (*s >= '0' && *s <= '9'){
            padding = 10*padding + (*s - '0');
            ++s;
        }
        if(*s == '%') {
            putc('%', false);
            ++s;
        } else if(*s == 'u') {
            printUInt(va_arg(ap, uint), 10, padding, false);
            ++s;
        } else if(*s == 'd') {
            printInt(va_arg(ap, int), 10, padding, false);
            ++s;
        } else if(*s == 'x'|| *s == 'p') {
            //putc('0', false); // wrong relatively to the standard
            //putc('x', false);
            printUInt(va_arg(ap, int), 16, padding, false);
            ++s;
        } else if(*s == 's') {
            puts(va_arg(ap, char*), false);
            ++s;
        } else if(*s == 'c') {
            //integer types smaller than int are promoted to int
            //when used in a ...
            char c = va_arg(ap, int);
            putc(c, false);
            ++s;
        } else {
            //omg me dunno wat to do!!!
        }
    }
    updateCursor();
}

void FrameBuffer::printf(const char* s, ...) {
    va_list ap;
    va_start(ap, s);
    vprintf(s, ap);
    va_end(ap);
}

