#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>
#include <stdint.h>// int32 etc
#include <stdarg.h>

#ifdef SUP_OS_KERNEL
#include <vector>
#include <string>
#endif


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


typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef intptr_t iptr;
typedef uintptr_t uptr;

static_assert(sizeof(int8_t)==1);
static_assert(sizeof(uint8_t)==1);
static_assert(sizeof(int16_t)==2);
static_assert(sizeof(uint16_t)==2);
static_assert(sizeof(int32_t)==4);
static_assert(sizeof(uint32_t)==4);
static_assert(sizeof(int64_t)==8);
static_assert(sizeof(uint64_t)==8);
#ifdef SUP_OS_KERNEL
static_assert(sizeof(size_t)==8);
#endif
static_assert(sizeof(iptr) == sizeof(void*));
static_assert(sizeof(uptr) == sizeof(void*));


void outb(u16 port, u8 data);
u8 inb(u16 port);
void outw(u16 port, u16 data);
u16 inw(u16 port);

void wrmsr(u32 num,u64 value); // read and write MSR from C
u64 rdmsr(u32 num);

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

inline void* getCR2(){
    void* res;
    asm("mov %%cr2,%0" :
        "=r"(res)
        :
        :
        );
    return res;
}

inline void fpu_load_control_word(u16 control) {
    asm volatile("fldcw %0;"::"m"(control));
}

[[noreturn]] void vbsod(const char* s, va_list ap);

/**
   @brief Do a Blue Screen Of Death.

   It is a printf-like function.
 */
extern "C" [[noreturn]]  void bsod(const char* s, ...);

/**
   @brief Reboot the kernel.

   Ugly hack that triple fault the processor.
 */
void reboot();

template<typename T, typename U> T alignup(T n, U multiple) {
    return ((n+multiple-1)/multiple)*multiple;
}

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
#define barrier asm volatile("" : : : "memory")


#define cli asm volatile("cli");
#define sti asm volatile("sti");
#define stop asm volatile("xchg %bx,%bx ; cli ; hlt");


#define WAIT(time) do { \
        for(volatile u64 i = 0 ; i < time ; ++i); \
    } while(false)


const u64 HHOFFSET = -0x80000000; //High half kernel offset ~ 250T
static_assert((HHOFFSET & 0x3FFFFFFF) == 0);

void pbool(bool b,const char* = "");

#ifdef SUP_OS_KERNEL
std::vector<std::string> split(std::string str,char separator,bool keepEmpty = true);
std::string concat(std::vector<std::string> strs,char separator);

void kloop();
[[noreturn]]void kend(); // close the kernel

#endif

#endif

