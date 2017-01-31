#ifndef PIC_H
#define PIC_H

#include "utility.h"

class Pic {
    public:
        Pic();
        void endOfInterrupt(uchar irq);
        ushort getMask();
        void setMask(ushort mask);
    private:
        ushort _mask;
};

#endif

