#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utility.h"

class InterruptParams;

struct Context{
    void* other; // pointer to save x87/SSE/MMX state later
    u64 r15;
    u64 r14; // 0x10
    u64 r13;
    u64 r12; //0x20
    u64 r11;
    u64 r10; //0x30
    u64 r9;
    u64 r8;  //0x40
    u64 rdi;
    u64 rsi; //0x50
    u64 rbp;
    u64 rsp; //0x60
    u64 rdx;
    u64 rax; //0x70
    u64 rcx;
    u64 rflags; //0x80
    u64 rbx; // simpler for interrupts
    u64 rip; //0x90
    static Context* lastContext; // lastContext maybe the stack save of interrupt state,
    // any modification to its content means that you can't return normally
    // from interrupt or system call.

    explicit Context(InterruptParams* params);
    [[noreturn]] void launchlast(){lastContext->launch();}
    [[noreturn]] void launch();
    void save(){
        lastContext = this;
    }
    static void save(InterruptParams* params);
}__attribute__((packed));


#endif
