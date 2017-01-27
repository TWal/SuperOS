#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

class FrameBuffer {
    public:
        FrameBuffer();
        void moveCursor(int col, int row);
        void writeChar(char c, int col, int row, char fg = 15, char bg = 0);
        void clear();
    private:
        int _cursCol;
        int _cursRow;
};

#endif

