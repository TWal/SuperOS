#include "Pit.h"

#define PIT_BASE 0x40

void Pit::set(u8 channel, u16 divisor,u8 mode){
    PitCommand pc;
    pc.BCD = false;
    pc.mode = mode;
    pc.accessMode = LOHIBYTE;
    pc.channel = channel;
    outb(PIT_BASE +3,pc);
    outb(PIT_BASE,(u8)divisor);
    outb(PIT_BASE,u8(divisor >> 8));
}
