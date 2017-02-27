#ifndef INTERRUPT_H
#define INTERRUPT_H

#include"utility.h"


struct InterruptEntry {
    ushort offLow;
    ushort segSelector;
    char reserved0;
    char reserved1 :3;
    bool is32bits : 1;
    char reserved2 :1;
    char privilegeLvl :2;
    bool present : 1;
    ushort offHigh;

    InterruptEntry();
    void setAddr(void* addr);
    void* getAddr();

}__attribute__((packed));
static_assert(sizeof(InterruptEntry) == 8 , "InterruptEntry has wrong size");

extern InterruptEntry IDT[256];

void lidt();

struct InterruptParams{
    uint32 eax;
    uint32 ebx;
    uint32 ecx;
    uint32 edx;
    uint32 interNumTimes4;
    uint32 esi;
    uint32 ebp;
    uint32 edi;
    void*  eip;
    uint32 cs;
    uint32 flags;
}__attribute__((packed));

struct InterruptParamsErr{
    uint32 eax;
    uint32 ebx;
    uint32 ecx;
    uint32 edx;
    uint32 interNumTimes4;
    uint32 esi;
    uint32 ebp;
    uint32 edi;
    uint32 errorCode;
    void*  eip;
    uint32 cs;
    uint32 flags;
}__attribute__((packed));

struct IntParams{
    bool isAssembly : 1;
    bool doReturn : 1;
    bool hasErrorCode : 1;
    IntParams(): isAssembly(false),doReturn(false),hasErrorCode(false){};

}__attribute__((packed));


typedef uint (*interFuncR)(const InterruptParams);
typedef void (*interFunc)(const InterruptParams);
typedef uint (*interFuncER)(const InterruptParamsErr);
typedef void (*interFuncE)(const InterruptParamsErr);

extern "C" interFunc intIDT[256];
extern "C" void initIntIDT ();
extern "C" IntParams params [256]; // switch to bitset when libc++ is avaible

class InterruptTable {
public:
    void init();
    void addInt(int i,interFunc f);
    void addInt(int i,interFuncR f);
    void addInt(int i,interFuncE f);
    void addInt(int i,interFuncER f);
    void addIntAsm(int i, void* f);// asm handler
    InterruptTable();
    void allPresent();// only for debugging purpose
};



template<uchar num>
void interrupt(){
    asm volatile(
        "int %0;" :
        : "i"(num) :
        );
}

template<uchar num>
void interrupt(int eax){
    asm volatile(
        "mov %1,%%eax;"
        "int %0;" :
        : "i"(num) , "r"(eax): "%eax"
        );
}

template<uchar num>
void interrupt(int eax,int ebx){
    asm volatile(
        "mov %1,%%eax;"
        "mov %2,%%ebx;"
        "int %0;" :
        : "i"(num) , "r"(eax),"r"(ebx)
        : "%eax", "%ebx"
        );
}

template<uchar num>
int interruptr(){
    int res;
    asm volatile(
        "int %1;"
        "mov %%eax,%0;"
        :"=r"(res)
        :"i"(num)
        :"%eax"
        );
    return res;
}

template<uchar num>
int interruptr(int eax){
    int res;
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

template<uchar num>
int interruptr(int eax,int ebx){
    int res;
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



#endif
