#include "../utility.h"
#include "Syscall.h"
#include <stdio.h>
#include"../Interrupts/Interrupt.h"
#include "Context.h"

#define STAR 0xC0000081
#define LSTAR 0xC0000082
#define CSTAR 0xC0000083
#define SFMASK 0xC0000084

extern "C" void syscall(); // syscall asm handler : save register then call the other one

syscallHandler systable[SYSCALLNUM+1];

syscallHandler * const handlers = systable +1; // handlers[-1] is thus valid

void syserror(){
    printf("Current program has made an invalid syscall\n");
    // call exit(1) , do not return.
    stop;
}

u64 systest(u64 a,u64 b,u64 c,u64 d,u64 e,u64 f){
    printf("Systest called with %lld, %lld, %lld, %lld, %lld, %lld \n",a,b,c,d,e,f);
    printf("And rsp at %p and rip at %p\n",Context::lastContext->rsp,Context::lastContext->rip);
    Context::lastContext->launch(); // same behavior than normal return
    return 0;
}

u64 syscallInt(const InterruptParams& par){
    syscallHandler hand = nullptr;
    if(par.rax > 255) hand = handlers[SYSERROR];
    else if(handlers[par.rax]) hand = handlers[par.rax];
    else hand = handlers[SYSERROR];

    return hand(par.rbx,par.rcx,par.rdx,par.rsi,par.rdi,par.rbp);
}

void syscallInit(){
    wrmsr(STAR,(0x1bull << 48) + (0x8ull << 32)); // set segment of OS and usermode
    // 0x8 is OS segment, 0xb +16 is user modesegment (+16 because return in 64 bits mode)
    wrmsr(LSTAR,(u64)syscall);
    wrmsr(CSTAR,0);
    wrmsr(SFMASK,0); // we keep the flags active TODO : think if it is OK

    handlers[-1] = reinterpret_cast<syscallHandler>(syserror);
    for(int i = 0 ; i < SYSCALLNUM ; ++ i){
        handlers[i] = nullptr;
    }

    handlers[SYSTEST] = systest;

    idt.addInt(0x80,syscallInt);

    // Other pieces of code should register their handlers AFTER calling this function.
}

extern "C" void syssave(){
    Context::lastContext = (Context*)-sizeof(Context);
}





