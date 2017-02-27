#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>// int32 etc
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


typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

static_assert(sizeof(int8_t)==1);
static_assert(sizeof(uint8_t)==1);
static_assert(sizeof(int16_t)==2);
static_assert(sizeof(uint16_t)==2);
static_assert(sizeof(int32_t)==4);
static_assert(sizeof(uint32_t)==4);
static_assert(sizeof(int64_t)==8);
static_assert(sizeof(uint64_t)==8);


void outb(uint16 port, uchar data);
uchar inb(uint16 port);
void outw(uint16 port, ushort data);
ushort inw(uint16 port);

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

std::vector<std::string> split(std::string str,char separator,bool keepEmpty = true);
std::string concat(std::vector<std::string> strs,char separator);

#define WAIT(time) do { \
        for(volatile int i = 0 ; i < time ; ++i); \
    } while(false)

#endif

