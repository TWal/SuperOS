
#ifdef SUP_OS_KERNEL
#include "utility.h"
#include "IO/FrameBuffer.h"
#include "Interrupts/Interrupt.h"
#include "Graphics/Screen.h"
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



u64 rdmsr(u32 num);

#ifdef SUP_OS_KERNEL
static void drawText(const char* s, video::Vec2u pos, const video::Color& fg, const video::Color& bg) {
    video::Vec2u size = video::screen.getSize();
    while(*s) {
        video::screen.putChar(*s, pos.x, pos.y, *video::Font::def, fg, bg);
        pos.x += 8;
        if(pos.x + 8 >= size.x) {
            pos.x = 0;
            pos.y += 16;
        }
        if(pos.y >= size.y) {
            return;
        }
        ++s;
    }
}

static video::Color blueline[2000];
[[noreturn]]void vbsod(const char* s, va_list ap) {
    cli;
    vfprintf(stderr,s,ap);
    if(!video::screen.isOK()) stop;
    video::Color bg({142, 0, 0});
    video::Color fg({200, 200, 200});
    for(int i = 0; i < 2000; ++i) {
        blueline[i] = bg;
    }

    video::Vec2u size = video::screen.getSize();
    for(uint y = 0; y < size.y; ++y) {
        video::screen.writeLine(y, 0, size.x, blueline);
    }

    const char* textbsod = "The Blue Screen Of Death";
    const char* texttm = "(tm)";
    const char* texterror = "Me haz an error!!!";
    const char* textlost = "(btw, you lost)";

    video::Vec2u bsodpos = {size.x/2 - 8*(strlen(textbsod)/2), size.y/8};
    drawText(textbsod, bsodpos, fg, bg);

    video::Vec2u tmpos = {bsodpos.x + 8*strlen(textbsod), bsodpos.y-8};
    drawText(texttm, tmpos, fg, bg);

    video::Vec2u lostpos = {size.x - 8*strlen(textlost) - 1, size.y-16 - 1};
    drawText(textlost, lostpos, fg, bg);

    video::Vec2u errorpos = {size.x/20, 2*size.y/8};
    drawText(texterror, errorpos, fg, bg);

    video::Vec2u spos = {errorpos.x, errorpos.y + 16};
    drawText(s, spos, fg, bg);

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

