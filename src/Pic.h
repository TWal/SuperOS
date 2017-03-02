#ifndef PIC_H
#define PIC_H

#include "utility.h"

class Pic {
    public:
        Pic();
        void endOfInterrupt(u8 irq);
        u16 getMask();
        void setMask(u16 mask);
        void activate(u8 irq);
        void desactivate(u8 irq);
        //From http://wiki.osdev.org/Interrupts#Standard_ISA_IRQs
        enum IRQ {
            TIMER = 0,
            KEYBOARD,
            SLAVE_PIC,
            COM2,
            COM1,
            LPT2,
            FLOPPY,
            LPT1,
            RTC,
            GEN_IO_1,
            GEN_IO_2,
            GEN_IO_3,
            MOUSE,
            FPU,
            IDE1,
            IDE2
        };
    private:
        ushort _mask;
};

#endif

