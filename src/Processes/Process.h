#ifndef PROCESS_H
#define PROCESS_H

#include "../utility.h"
#include "../Memory/Paging.h"
#include <vector>
#include "FileDescriptor.h"
#include "../Bytes.h"
#include "../Memory/Heap.h"
#include "../Interrupts/Interrupt.h"

#define FIXED_SIZE_STACK 1

struct GeneralRegisters{
    u64 rax;
    u64 rbx;
    u64 rcx;//10
    u64 rdx;
    u64 rsp;//20
    u64 rbp;
    u64 rsi;//30
    u64 rdi;
    u64 r8; //40
    u64 r9;
    u64 r10;//50
    u64 r11;
    u64 r12;//60
    u64 r13;
    u64 r14;//70
    u64 r15;
};

class Thread;

class Process{
    u32 _pid;
    u32 _threadNum;
    // Descriptor table
    bool _terminated; // i.e zombie
    u64 _returnCode; // valid iff terminated = true
    std::vector<FileDescriptor*> _fds;
    void * _userPDP;
    std::vector<Thread*> _threads;
    //Heap* heap;
public :
    Process(u32 pid,std::vector<FileDescriptor*> fds = std::vector<FileDescriptor*>() );
    ~Process();
    // load the elf64 file Bytes and create the main thread starting on its entry point.
    Thread* loadFromBytes(Bytes* file);
    void clear(); // clear the process : no thread, clean mapped memory
    u32 getPid(){return _pid;}
    void addThread(Thread* thread);
    void terminate(u64 returnCode);
    void prepare();
};

class WaitingReason;

class Thread{
    u64 _rip;
    u64 _rflags;
    GeneralRegisters _registers; // the thread stack is contained here
    Process* _process;
public :
    Thread(u64 rip,Process* parent);
    WaitingReason* wr; // if wr == nullptr, the thread is runnable.
    [[noreturn]] void run(); // launch the thread until the next timer interruption
    Process* getProcess(){return _process;}
    void storeContext(const InterruptParams& params);

};

#endif
