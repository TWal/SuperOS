#include "Pic.h"

static const ushort PIC1 = 0x20;
static const ushort PIC2 = 0xA0;
static const ushort PIC1_COMMAND = PIC1;
static const ushort PIC1_DATA    = PIC1+1;
static const ushort PIC2_COMMAND = PIC2;
static const ushort PIC2_DATA    = PIC2+1;

Pic::Pic() {
    //see http://web.archive.org/web/20041023230527/http://users.win.be/W0005997/GI/pic.html
    const uchar ICW1_ICW4	= 0x01;
    //const uchar ICW1_SINGLE	= 0x02;
    //const uchar ICW1_LEVEL	= 0x08;
    const uchar ICW1_INIT	= 0x10;
    const uchar ICW4_8086 = 0x01;
    //const uchar ICW4_AUTO = 0x02;

    //ICW1
    const uchar ICW1 = ICW1_INIT | ICW1_ICW4;
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);

    const uchar OFFSET1 = 0x20;
    const uchar OFFSET2 = 0x28;
    //ICW2
    outb(PIC1_DATA, OFFSET1);
    outb(PIC2_DATA, OFFSET2);

    //ICW3
    outb(PIC1_DATA, 0x4);
    outb(PIC2_DATA, 0x2);


    //ICW4
    const uchar ICW4 = ICW4_8086;
    outb(PIC1_DATA, ICW4);
    outb(PIC2_DATA, ICW4);

    setMask(0xFFFD);
    asm volatile("sti");
}

void Pic::endOfInterrupt(uchar irq) {
    const uchar PIC_EOI = 0x20;
    if(irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

ushort Pic::getMask() {
    return _mask;
}

void Pic::setMask(ushort mask) {
    _mask = mask;
    outb(PIC1_DATA, _mask&0x00FF);
    outb(PIC2_DATA, _mask&0xFF00 >> 8);
}

