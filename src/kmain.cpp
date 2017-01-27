#include "FrameBuffer.h"

extern "C" void kmain() {
    FrameBuffer fb;
    const char* s = "Hello World!";
    for(int i = 0; i < 12; ++i) {
        fb.writeChar(s[i], i, i);
    }
}
