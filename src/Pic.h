#ifndef PIC_H
#define PIC_H

#include "utility.h"

class Pic {
    public:
        Pic();
        void endOfInterrupt(uchar irq);
        void setMask(uchar irq);
        void clearMask(uchar irq);
};

#endif

