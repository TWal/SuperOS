
#ifdef SUP_OS_KERNEL
#include "utility.h"
#include "IO/FrameBuffer.h"
#include "Interrupts/Interrupt.h"
#include "Graphics/Screen.h"
#include "IO/OSFileDesc.h"
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

void wrmsr(u32 num,u64 value){
    asm volatile(
        "wrmsr" :
        :
        "c"(num),"a"((u32)value),"d"((u32)(value >> 32)) :
        //"%rdx"
        );
}

u64 rdmsr(u32 num){
    u32 resh,resl;
    asm volatile(
        "rdmsr;" :
        "=a"(resl),"=d"(resh) :
        "c"(num) :
        );
    return (u64)resl + ((u64)resh << 32);
}



#ifdef SUP_OS_KERNEL
static video::Vec2u drawText(const char* s, size_t count, video::Vec2u pos, const video::Color& fg, const video::Color& bg, uint minX, uint maxX) {
    video::Vec2u size = video::screen.getSize();
    for(size_t i = 0; i < count; ++i) {
        if(pos.y >= size.y) {
            return pos;
        }
        if(s[i] == '\n') {
            pos.x = minX;
            pos.y += 16;
            continue;
        }
        video::screen.putChar(s[i], pos.x, pos.y, *video::Font::def, fg, bg);
        pos.x += 8;
        if(pos.x + 8 >= maxX) {
            pos.x = minX;
            pos.y += 16;
        }
    }

    return pos;
}

class BsodStream : public Stream {
    public:
        BsodStream(uint minX, uint maxX, uint startY, video::Color fg, video::Color bg) :
            _pos(minX, startY), _minX(minX), _maxX(maxX), _fg(fg), _bg(bg) {}

        u64 getMask() const {
            return Stream::WRITABLE | Stream::APPENDABLE;
        }

        size_t write(const void* buf,size_t count) {
            _pos = drawText((char*)buf, count, _pos, _fg, _bg, _minX, _maxX);
            OSStreams[2]->write(buf, count);
        }

    private:
        video::Vec2u _pos;
        uint _minX;
        uint _maxX;
        video::Color _fg;
        video::Color _bg;
};

void drawRectangle(uint thickness, const video::Vec2u& upleft, const video::Vec2u& downright, const video::Color& color) {
    for(uint x = upleft.x; x <= downright.x; ++x) {
        for(uint y = 0; y < thickness; ++y) {
            video::screen.set(x, upleft.y+y, color);
            video::screen.set(x, downright.y-y, color);
        }
    }
    for(uint y = upleft.y; y <= downright.y; ++y) {
        for(uint x = 0; x < thickness; ++x) {
            video::screen.set(upleft.x + x, y, color);
            video::screen.set(downright.x - x, y, color);
        }
    }

}

static video::Color blueline[2000];
[[noreturn]]void vbsod(const char* s, va_list ap) {
    cli;
    if(!video::screen.isOK()) stop;
    const uint THICKNESS = 10;
    video::Color bg({142, 0, 0});
    video::Color fg({200, 200, 200});
    for(int i = 0; i < 2000; ++i) {
        blueline[i] = bg;
    }

    video::Vec2u size = video::screen.getSize();
    video::Vec2u start = {3*THICKNESS, 3*THICKNESS};
    video::Vec2u trueSize = size;
    size -= video::Vec2u(6*THICKNESS, 6*THICKNESS);

    for(uint y = 0; y < trueSize.y; ++y) {
        video::screen.writeLine(y, 0, trueSize.x, blueline);
    }

    const char* textbsod = "The Blue Screen Of Death";
    const char* texttm = "(tm)";
    const char* textlost = "(btw, you lost)";

    video::Vec2u bsodpos = {size.x/2 - 8*uint(strlen(textbsod)/2), size.y/8};
    drawText(textbsod, 24, start+bsodpos, fg, bg, start.x, start.x+size.x);

    video::Vec2u tmpos = {bsodpos.x + 8*(uint)strlen(textbsod), bsodpos.y-8};
    drawText(texttm, 4, start+tmpos, fg, bg, start.x, start.x+size.x);

    video::Vec2u lostpos = {size.x - 8*(uint)strlen(textlost) - 1 - 10, size.y-16 - 1 - 10};
    drawText(textlost, 15, start+lostpos, fg, bg, start.x, start.x+size.x);

    BsodStream bs(start.x + size.x/20, start.x + size.x - (size.x/20), start.y + size.y/4, fg, bg);
    OSStreams[1] = &bs;
    printf("Me haz an error!!!\n");
    vfprintf(stdout, s, ap);

    drawRectangle(THICKNESS, {0, 0}, {trueSize.x-1, trueSize.y-1}, video::Color({64, 20, 5}));
    drawRectangle(THICKNESS, {THICKNESS, THICKNESS}, {trueSize.x-THICKNESS-1, trueSize.y-THICKNESS-1}, video::Color({255, 255, 255}));
    drawRectangle(THICKNESS, {2*THICKNESS, 2*THICKNESS}, {trueSize.x-2*THICKNESS-1, trueSize.y-2*THICKNESS-1}, video::Color({32, 25, 236}));
    video::screen.send();

    while(true) asm volatile("hlt");

#if 0
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
#endif
}
#else
[[noreturn]]void vbsod(const char*, va_list) {
    while(true) asm volatile("hlt");
}
#endif

[[noreturn]]void bsod(const char* s, ...) {
    va_list ap;
    va_start(ap, s);
    vbsod(s, ap);
    va_end(ap);
}

#ifdef SUP_OS_KERNEL
[[noreturn]]void reboot() {
    IDT[0].present = false; //remove div0
    IDT[8].present = false; //remove double fault;
    volatile int i = 0;
    volatile int j = 42/i;
    (void)j;
    while(true);
}

vector<string> split(std::string str,char separator,bool keepEmpty){
    vector<string> res ;
    size_t pos = 0;
    while (pos < str.size()){
        size_t pos2 = str.find_first_of(separator,pos);
        if (pos2 == string::npos){
            res.push_back(str.substr(pos));
            break;
        }
        if (keepEmpty || pos2 > pos){
            res.push_back(str.substr(pos,pos2 - pos));
        }
        pos = pos2 +1;
    }
    return res;
}

std::string concat(std::vector<std::string> strs,char separator){
    string res;
    for(auto& s : strs){
        res.append(s);
        res.push_back(separator);
    }
    res.pop_back();
    return res;
}
#endif

void pbool(bool b,const char* s){
    if(b)kprintf("%s : OK\n",s);
    else kprintf("%s : Not OK\n",s);
}

