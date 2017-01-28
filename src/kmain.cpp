#include "FrameBuffer.h"
#include "utility.h"

extern "C" void kmain() {
    FrameBuffer fb;
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < i; ++j) {
            fb.putc(' ');
        }
        fb.printf("%s%c %d\f", "Hello World", '!', i);
    }

    while(true) {
        for(int i = 0; i < 5000000; ++i);
        fb.scroll(1);
    }
}
