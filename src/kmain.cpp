#include "FrameBuffer.h"
#include "utility.h"

extern "C" void kmain() {
    FrameBuffer fb;
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < i; ++j) {
            fb.putc(' ');
        }
        fb.puts("Hello World! ");
        fb.printInt(i, 10);
        fb.putc('\f');
    }

    while(true) {
        for(int i = 0; i < 5000000; ++i);
        fb.scroll(1);
    }
}
