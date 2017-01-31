#ifndef PIC_H
#define PIC_H

#include "utility.h"

class Pic {
    public:
        Pic();
        void endOfInterrupt(uchar irq);
        ushort getMask();
        void setMask(ushort mask);
        void activate(uchar irq);
        void desactivate(uchar irq);
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

