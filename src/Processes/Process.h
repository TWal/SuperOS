#ifndef PROCESS_H
#define PROCESS_H

#include "../utility.h"
#include "../Memory/Paging.h"

struct GeneralRegisters{
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;
    u64 rsp;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
};


class Process{
    u32 pid;
    u32 threadNum;
    PageEntry * userPDP;
    // Descriptor table
    bool terminated; // i.e zombie
    u64 returnCode; // valid iff terminated = true
};

class WaitingReason;

class Thread{
    u64 rip;
    GeneralRegisters registers; // the thread stack is contained here
    Process* process;
    WaitingReason* wr; // if wr == nullptr, the thread is runnable.
    void run(); // launch the thread until the next timer interruption
};

#endif
