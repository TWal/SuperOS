#ifndef UTILITY_H
#define UTILITY_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;


void outb(ushort port, uchar data);
int inb(ushort port);

template<typename T> T min(const T& a, const T& b) {
    if(a < b) {
        return a;
    } else {
        return b;
    }
}

template<typename T> T max(const T& a, const T& b) {
    if(a < b) {
        return b;
    } else {
        return a;
    }
}

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l) __builtin_va_arg(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_copy(v1,v2) __builtin_va_copy(v1,v2)
typedef __builtin_va_list va_list;

#endif

