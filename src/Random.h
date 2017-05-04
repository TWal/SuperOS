#ifndef RANDOM_H
#define RANDOM_H

#include "utility.h"

class Random {
    public:
        enum {
            // Assumes W = 32 (omitting this)
            N = 624,
            M = 397,
            R = 31,
            A = 0x9908B0DF,

            F = 1812433253,

            U = 11,
            // Assumes D = 0xFFFFFFFF (omitting this)

            S = 7,
            B = 0x9D2C5680,

            T = 15,
            C = 0xEFC60000,

            L = 18,

            MASK_LOWER = (1ull << R) - 1,
            MASK_UPPER = (1ull << R)
        };


        Random();
        void seed(u32 seed);
        u32 rand();

    private:
        void _twist();
        u32  _mt[N];
        u16  _index;
};

extern Random random;

#endif

