#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "utility.h"

class FrameBuffer {
    public:
        FrameBuffer();
        void clear(char fg = 0xF, char bgj= 0x0);
        void moveCursor(int col, int row, bool updateCurs=true);
        void updateCursor();
        void writeChar(char c, int col, int row, char fg = 15, char bg = 0);
        void scroll(uint n, bool updateCurs=true);
        void putc(char c, bool updateCurs=true);
        void puts(const char* s, bool updateCurs=true);
        void printDigit(int d, bool updateCurs=true);
        void printInt(int n, uint base, uint padding=0, bool updateCurs=true);
        void vprintf(const char* s, va_list ap);
        void printf(const char* s, ...);

        enum Color {
            BLACK = 0,
            BLUE,
            GREEN,
            CYAN,
            RED,
            MAGENTA,
            BROWN,
            LIGHTGREY,
            DARKGREY,
            LIGHTBLUE,
            LIGHTGREEN,
            LIGHTCYAN,
            LIGHTRED,
            LIGHTMAGENTA,
            LIGHTBROWN,
            WHITE
        };

    private:
        int _cursCol;
        int _cursRow;
        char _fg;
        char _bg;
};

#endif

