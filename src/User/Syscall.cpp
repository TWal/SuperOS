#include "../utility.h"
#include "Syscall.h"
#include <stdio.h>
#include"../Interrupts/Interrupt.h"
#include "Context.h"
#include "../Processes/Scheduler.h"

#define STAR 0xC0000081
#define LSTAR 0xC0000082
#define CSTAR 0xC0000083
#define SFMASK 0xC0000084

extern "C" void syscall(); ///< Syscall asm handler : save registers then calls handlers[rax]

syscallHandler systable[SYSCALLNUM+1];

syscallHandler * const handlers = systable +1; // handlers[-1] is thus valid


/// Handler for syscall -1.
void syserror(){
    bsod("Current program has made an invalid syscall\n");
    // call exit(1) , do not return.
    stop;
}

/// Test handler for debbuging, currently syscall 42.
u64 systest(u64 a,u64 b,u64 c,u64 d,u64 e,u64 f){
    printf("Systest called with %lld ",a,b,c,d,e,f);
    printf("And rsp at %p and rip at %p\n",Context::lastContext->rsp,Context::lastContext->rip);
    //Context::lastContext->launch(); // same behavior than normal return
    return 0;
}


extern "C" void sysleave(u64 ret);
/// @brief Interrupt handlers for interrupt 0x80
u64 syscallInt(const InterruptParams& par){
    syscallHandler hand = nullptr;
    if(par.rax > 255) hand = handlers[SYSERROR];
    else if(handlers[par.rax]) hand = handlers[par.rax];
    else hand = handlers[SYSERROR];
    Context::save(par); // saving context.
    u64 a = hand(par.rbx,par.rcx,par.rdx,par.rsi,par.rdi,par.rbp);
    sysleave(a);
    return a;
}

/**
   @brief Syscall system initialization.

   This function also clear handlers so you should only setup syscalls
   after it has been called.
*/
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
    idt.setTrapGate(0x80);
}

/// Save current syscall context in Context::lastContext.
extern "C" void syssave(){
    Context::lastContext = (Context*)-sizeof(Context);
}

extern "C" void sysleave(u64 ret){
    if(!schedul.canReturn()){
        schedul.setRet(ret);
        schedul.stopCurent();
        if(schedul.souldRender()){
            kloop();
            schedul.doneRender();
        }
        schedul.run();
    }
    else return;
}

