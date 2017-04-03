#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "../utility.h"


struct InterruptEntry {
    u16 offLow;
    u16 segSelector;
    char ist : 3;
    char zero :5;
    bool isTrap : 1;
    char ones :3;
    char zero2 : 1;
    char privilegeLvl :2;
    bool present : 1;
    u64 offHigh : 48;
    u32 zero3;

    InterruptEntry();
    void setAddr(void* addr);
    void* getAddr();

}__attribute__((packed));

static_assert(sizeof(InterruptEntry) == 16 , "InterruptEntry has wrong size");

extern InterruptEntry IDT[256];

void lidt();

struct InterruptParams{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    void*  rip;
    u64 cs;
    u64 flags;
    u64 oldrsp;
    u64 oldss;
}__attribute__((packed));

struct InterruptParamsErr{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    u64 errorCode;
    void*  rip;
    u64 cs;
    u64 flags;
    u64 oldrsp;
    u64 oldss;
}__attribute__((packed));

struct IntParams{
    bool isAssembly : 1;
    bool doReturn : 1;
    bool hasErrorCode : 1;
    IntParams(): isAssembly(false),doReturn(false),hasErrorCode(false){};
}__attribute__((packed));

static_assert(sizeof(IntParams) == 1, "IntParams is too big");

typedef u64 (*interFuncR)(const InterruptParams&);
typedef void (*interFunc)(const InterruptParams&);
typedef u64 (*interFuncER)(const InterruptParamsErr&);
typedef void (*interFuncE)(const InterruptParamsErr&);

extern "C" interFunc intIDT[256];
extern "C" void initIntIDT ();
extern "C" IntParams params [256]; // switch to bitset when libc++ is avaible

class InterruptTable {
public:
    void init();
    void addInt(u8 i,interFunc f);
    void addInt(u8 i,interFuncR f);
    void addInt(u8 i,interFuncE f);
    void addInt(u8 i,interFuncER f);
    void addIntAsm(u8 i, void* f);// asm handler
    InterruptTable();
    void allPresent();// only for debugging purpose
};



template<u8 num>
void interrupt(){
    asm volatile(
        "int %0;" :
        : "i"(num) :
        );
}

template<u8 num>
void interrupt(u64 rax){
    asm volatile(
        "mov %1,%%rax;"
        "int %0;" :
        : "i"(num) , "r"(rax): "%rax"
        );
}

template<u8 num>
void interrupt(u64 rax,u64 rbx){
    asm volatile(
        "mov %1,%%rax;"
        "mov %2,%%rbx;"
        "int %0;" :
        : "i"(num) , "r"(rax),"r"(rbx)
        : "%rax", "%rbx"
        );
}

template<u8 num>
u64 interruptr(){
    u64 res;
    asm volatile(
        "int %1;"
        "mov %%rax,%0;"
        :"=r"(res)
        :"i"(num)
        :"%rax"
        );
    return res;
}

template<u8 num>
u64 interruptr(u64 rax){
    u64 res;
    asm volatile(
        "mov %2,%%rax;"
        "int %1;"
        "mov %%rax,%0;"
        :"=r"(res)
        :"i"(num), "r"(rax)
        :"%rax"
        );
    return res;
}

template<u8 num>
u64 interruptr(u64 rax,u64 rbx){
    u64 res;
    asm volatile(
        "mov %2,%%rax;"
        "mov %3,%%rbx;"
        "int %1;"
        "mov %%rax,%0"
        :"=r"(res)
        : "i"(num) , "r"(rax),"r"(rbx)
        : "%rax", "%rbx"
        );
    return res;
}

extern InterruptTable idt;



#endif
