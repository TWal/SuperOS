#ifndef UTILITY_H
#define UTILITY_H

#include <stdarg.h>
#include <vector>
#include <string>


typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long int lint;
typedef unsigned long long int ulint;

static_assert(sizeof(char) == 1);
static_assert(sizeof(uchar) == 1);
static_assert(sizeof(short) == 2);
static_assert(sizeof(ushort) == 2);
static_assert(sizeof(int) == 4);
static_assert(sizeof(uint) == 4);
static_assert(sizeof(lint) == 8);
static_assert(sizeof(ulint) == 8);


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



void vbsod(const char* s, va_list ap);
extern "C" void bsod(const char* s, ...);

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

std::vector<std::string> split(std::string str,char separator);
std::string concat(std::vector<std::string> strs,char separator);

#define WAIT(time) do { \
        for(volatile int i = 0 ; i < time ; ++i); \
    } while(false)

#endif

