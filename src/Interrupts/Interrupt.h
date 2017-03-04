#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "../utility.h"


struct InterruptEntry {
    u16 offLow;
    u16 segSelector;
    char reserved0;
    char reserved1 :3;
    bool is32bits : 1;
    char reserved2 :1;
    char privilegeLvl :2;
    bool present : 1;
    u16 offHigh;

    InterruptEntry();
    void setAddr(void* addr);
    void* getAddr();

}__attribute__((packed));
static_assert(sizeof(InterruptEntry) == 8 , "InterruptEntry has wrong size");

extern InterruptEntry IDT[256];

void lidt();

struct InterruptParams{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 interNumTimes4;
    u32 esi;
    u32 ebp;
    u32 edi;
    void*  eip;
    u32 cs;
    u32 flags;
}__attribute__((packed));

struct InterruptParamsErr{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 interNumTimes4;
    u32 esi;
    u32 ebp;
    u32 edi;
    u32 errorCode;
    void*  eip;
    u32 cs;
    u32 flags;
}__attribute__((packed));

struct IntParams{
    bool isAssembly : 1;
    bool doReturn : 1;
    bool hasErrorCode : 1;
    IntParams(): isAssembly(false),doReturn(false),hasErrorCode(false){};

}__attribute__((packed));


typedef u32 (*interFuncR)(const InterruptParams&);
typedef void (*interFunc)(const InterruptParams&);
typedef u32 (*interFuncER)(const InterruptParamsErr&);
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
void interrupt(u32 eax){
    asm volatile(
        "mov %1,%%eax;"
        "int %0;" :
        : "i"(num) , "r"(eax): "%eax"
        );
}

template<u8 num>
void interrupt(u32 eax,u32 ebx){
    asm volatile(
        "mov %1,%%eax;"
        "mov %2,%%ebx;"
        "int %0;" :
        : "i"(num) , "r"(eax),"r"(ebx)
        : "%eax", "%ebx"
        );
}

template<u8 num>
u32 interruptr(){
    u32 res;
    asm volatile(
        "int %1;"
        "mov %%eax,%0;"
        :"=r"(res)
        :"i"(num)
        :"%eax"
        );
    return res;
}

template<u8 num>
u32 interruptr(u32 eax){
    u32 res;
    asm volatile(
        "mov %2,%%eax;"
        "int %1;"
        "mov %%eax,%0;"
        :"=r"(res)
        :"i"(num), "r"(eax)
        :"%eax"
        );
    return res;
}

template<u8 num>
u32 interruptr(u32 eax,u32 ebx){
    u32 res;
    asm volatile(
        "mov %2,%%eax;"
        "mov %3,%%ebx;"
        "int %1;"
        "mov %%eax,%0"
        :"=r"(res)
        : "i"(num) , "r"(eax),"r"(ebx)
        : "%eax", "%ebx"
        );
    return res;
}

extern InterruptTable idt;



#endif
