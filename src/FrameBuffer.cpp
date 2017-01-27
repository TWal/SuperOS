#include "FrameBuffer.h"

static char* const FB = (char*)0xB8000;

FrameBuffer::FrameBuffer() {
    clear();
}

void FrameBuffer::moveCursor(int col, int row) {
    (void)col;
    (void)row;
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
    for(int i = 0 ; i < 2 * 80 * 25 ; ++i){
        FB[i] = 0;
    }
}

