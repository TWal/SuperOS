#include "FrameBuffer.h"
#include "utility.h"

static char* const FB = (char*)0xB8000;

FrameBuffer::FrameBuffer() {
    clear();
}

void FrameBuffer::moveCursor(int col, int row) {
    static const ushort CURSOR_COMMAND_PORT = 0x3D4;
    static const ushort CURSOR_DATA_PORT = 0x3D5;
    static const ushort CURSOR_HIGH_BYTE = 14;
    static const ushort CURSOR_LOW_BYTE = 15;

    int i = 80*row + col;
    outb(CURSOR_COMMAND_PORT, CURSOR_HIGH_BYTE);
    outb(CURSOR_DATA_PORT, (i>>8) & 0xFF);
    outb(CURSOR_COMMAND_PORT, CURSOR_LOW_BYTE);
    outb(CURSOR_DATA_PORT, i & 0xFF);
}

void FrameBuffer::writeChar(char c, int col, int row, char fg, char bg) {
    int i = 80*row + col;
    FB[2*i] = c;
    FB[2*i+1] = (fg & 0xF) | ((bg & 0xF) << 4);
    (void)bg;
}

void FrameBuffer::clear() {
    _cursCol = 0;
    _cursRow = 0;
    for(int i = 0 ; i < 80 * 25 ; ++i){
        FB[2*i] = ' ';
        FB[2*i+1] = 15;
    }
}

