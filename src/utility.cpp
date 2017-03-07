
#ifdef SUP_OS_KERNEL
#include "utility.h"
#include "IO/FrameBuffer.h"
#else
#include "../src/utility.h"
#include "../src/IO/FrameBuffer.h"
#endif

using namespace std;

void outb(u16 port, u8 data) {
    asm volatile("outb %0, %1" : : "a"(data), "d"(port));
}

u8 inb(u16 port) {
    u8 res;
    asm volatile("inb %1; movb %%al, %0" : "=r"(res) : "d"(port) : "%al");
    return res;
}
void outw(u16 port, u16 data){
    asm volatile("outw %0, %1" : : "a"(data), "d"(port));
}
u16 inw(u16 port){
    u16 res;
    asm volatile("inw %1; movw %%ax, %0" : "=r"(res) : "d"(port) : "%ax");
    return res;
}


void vbsod(const char* s, va_list ap) {
    const char fg = FrameBuffer::WHITE;
    const char fgOut = FrameBuffer::LIGHTRED;
    const char bg = FrameBuffer::BLUE;
    FrameBuffer fb;
    fb.clear(fg, bg);
    fb.setMargin(3, 77);
    for(int j = 0; j < 2; ++j) {
        for(int i = j; i < 79-j; ++i) {
            fb.writeChar('~', i, 0+j, fgOut, bg);
            fb.writeChar('~', i+1, 24-j, fgOut, bg);
        }
    }
    for(int j = 0; j < 2; ++j) {
        for(int i = j; i < 24-j; ++i) {
            fb.writeChar('!', 0+j, i+1, fgOut, bg);
            fb.writeChar('!', 79-j, i, fgOut, bg);
        }
    }
    fb.moveCursor(26, 3);
    fb.puts("The Blue Screen Of Death (tm)");
    fb.moveCursor(3, 6);
    fb.puts("Me haz an error!!!");
    fb.moveCursor(62, 22);
    fb.puts("(btw, you lost)");
    fb.moveCursor(3,8);
    fb.vprintf(s, ap);

    breakpoint; //Don't waste cpu when using bochs!
    while(true) {
        asm volatile("cli;hlt");
    }
}

void bsod(const char* s, ...) {
    va_list ap;
    va_start(ap, s);
    vbsod(s, ap);
    va_end(ap);
}


