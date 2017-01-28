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

#endif

