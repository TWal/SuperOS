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

typedef int (*interFuncR)(const int,const int);
typedef void (*interFunc)(const int,const int);

extern "C" interFunc intIDT[256];
extern "C" void initIntIDT ();
extern "C" bool doReturn [256]; // switch when libc++ is avaible

class InterruptTable {
public:
    void init();
    void addInt(int i,interFunc f);
    void addInt(int i,interFuncR f);
    InterruptTable();
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
