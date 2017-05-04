#include "Random.h"

Random random;

Random::Random() {
    seed(42);
}

void Random::seed(u32 seed) {
    _mt[0] = seed;
    for(u32 i = 1; i < N; ++i) {
        _mt[i] = (F * (_mt[i-1] ^ (_mt[i-1] >> 30)) + i);
    }
    _index = N;
}

u32 Random::rand() {
    if(_index >= N) _twist();

    u32 y = _mt[_index];
    y ^= (_mt[_index] >> U);
    y ^= (y << S) & B;
    y ^= (y << T) & C;
    y ^= (y >> L);

    _index += 1;

    return y;
}

void Random::_twist() {
    for(u32 i = 0; i < N; ++i) {
        u32 x = (_mt[i] & MASK_UPPER) + (_mt[(i+1) % N] & MASK_LOWER);
        u32 xA = x >> 1;
        if(x & 0x1) xA ^= A;
        _mt[i] = _mt[(i + M) % N] ^ xA;
    }
    _index = 0;
}

