#include "../math.h"
#ifndef SUP_OS_KERNEL

double sin(double d){
    double res;
    asm(
        "fsin"
        : "=t"(res)
        : "0"(d)
        );
    return res;
}

double cos(double d){
    double res;
    asm(
        "fcos"
        : "=t"(res)
        : "0"(d)
        );
    return res;
}

double sqrt(double d){
    double res;
    asm(
        "fsqrt"
        : "=t"(res)
        : "0"(d)
        );
    return res;
}

#endif
