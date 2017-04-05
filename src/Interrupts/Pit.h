#ifndef PIT_H
#define PIT_H

#include "../utility.h"




struct PitCommand{
    bool BCD :1;
    u8 mode :3;
    u8 accessMode:2;
    u8 channel : 2;
    operator u8(){return *reinterpret_cast<u8*>(this);}
}__attribute__((packed));
static_assert(sizeof(PitCommand) == 1,"PitCommand has wrong size");


struct Pit{
    enum{ // Access Mode
        COUNT,LOBYTE,HIBYTE,LOHIBYTE
    };
    enum{ // mode
        ITC, // interrupt on terminal count
        RETRIGERABLE, // ITC + you can launch it input (only channel 2)
        RATE, // pulse at indicated frequency
        SQUAREWAVE, // explicit
    };
    static void set(u8 channel, u16 divisor,u8 mode);


};

#endif
