#ifndef UTILITY_H
#define UTILITY_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long int lint;


void outb(ushort port, uchar data);
uchar inb(ushort port);
void outw(ushort port, ushort data);
ushort inw(ushort port);

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

void vbsod(const char* s, va_list ap);
void bsod(const char* s, ...);

//Trick to have S__LINE _ a string containing the line number
#define __S(x) #x
#define __S_(x) __S(x)
#define S__LINE__ __S_(__LINE__)

#define assert(cond) { \
    if(!(cond)) { \
        bsod("Assertion failed at " __FILE__ ":" S__LINE__ ": " #cond); \
    } \
}

#define breakpoint asm volatile("xchg %bx, %bx")


#define cli asm volatile("cli");
#define sti asm volatile("sti");

#endif

