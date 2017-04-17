#ifndef CONTEXT_H
#define CONTEXT_H

#include"../utility.h"

class InterruptParams;

/**
   @brief This class represent a program's context

   It currently contains the value of all registers and flags and rip.

   @todo save x87/SSE/MMX state.

   The contained state should only by of a user mode program (rip and rsp must be positive)

 */

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
    /**
       @brief Store the last context in use.

       lastContext maybe the stack save of interrupt or syscall state.
       any modification to its content means that you can't return normally
       from interrupt or system call.

       This variable is automatically set on syscall by @ref syssave.
       It can also be setup by @ref save.

    */
    static Context* lastContext;

    Context(): other(nullptr){} ///< create an uninitialized context
    /**
       @brief Copy params into the context

       It check that this is a user mode context.
     */
    explicit Context(const InterruptParams& params);
    /// @copydoc Context(const InterruptParams&)
    Context& operator= (const InterruptParams& params);

    /// Launch last context.
    [[noreturn]] void launchlast(){lastContext->launch();}
    /**
       @brief Start executing the code from this context.

       UserMemory should have been setup before calling this function or
       it is UB.
     */
    [[noreturn]] void launch();


    /// Save current context in lastContext.
    void save(){
        lastContext = this;
    }
    /// Save params context in lastContext.
    static void save(const InterruptParams& params);
}__attribute__((packed));


#endif
