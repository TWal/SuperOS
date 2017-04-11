#include "Scheduler.h"
#include "../Interrupts/Pic.h"
#include "../Interrupts/Pit.h"
#include <stdio.h>
#include "../User/Syscall.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"

void* const tidBitset = (void*)(-0x80000000ll - 0x2000ll);


Scheduler::Scheduler() : _current(nullptr),RemainingTime(0){
    Pit::set(0,50000,Pit::SQUAREWAVE); // for now slow interruptions (~ 20 Hz)
    // we will speed up later when it's stable.

}
static u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}
static u64 sysfork(u64,u64,u64,u64,u64,u64){
    return schedul.fork();
}

void Scheduler::init(Thread* initThread){
    assert(initThread->getTid() == 1);

    //activate timer interrupts
    idt.addInt(0x20,::timerHandler);
    pic.activate(Pic::TIMER);

    //activate tid bitset;
    paging.createMapping((u64)physmemalloc.alloc(),(u64)tidBitset);
    _tids.init(tidBitset,0x1000*8);
    _tids.fill();
    _tids[0] = false; // no thread with number 0.
    _tids[1] = false;

    //declare scheduler syscalls :
    handlers[SYSEXIT] = sysexit;
    handlers[SYSFORK] = sysfork;

}

[[noreturn]] void Scheduler::run(){
    assert(!_current);
    assert(_threadFIFO.size() > 0 && "Empty Scheduler");

    printf("current queue is :");
    for(auto t : _threadFIFO){
        printf(" %d",t->getTid());
    }printf("\n");

    // pop the next candidate
    _current = _threadFIFO[0];
    _threadFIFO.pop_front();

    // If this thread has been deleted
    if(!_threads.count(_current->getTid())){ 
         _current = nullptr;
        run(); // stack will disappear no return cost.
    }
    // If this thread is waiting something (demo for now, not implemented)
    if(_current->wr){
        _threadFIFO.push_back((Thread*)_current);
        _current = nullptr;
        run();
    }

    // All is right, this thread can run
    RemainingTime = 5; // by default program runs for 5 ticks
    _current->run();

}
[[noreturn]] void Scheduler::exit(u64 returnCode){
    assert(_current);
    Process* pro = _current->getProcess();
    u16 tid = _current->getTid();
    _current = nullptr;
    pro->terminate(returnCode);

    if(pro->getPid() == 1){ // if init died
        printf("init died with code %lld",returnCode);
        kend(); // shutdown.
    }
    else{ // other cases
        delete pro;
        printf("%d died with %lld\n",tid,returnCode);
        run();
    }
}

u16 Scheduler::fork(){
    // we must be in a thread
    assert(_current);

    printf("Fork on instruction %p\n",_current->context.rip);

    // we save that thread context.
    _current->context = *Context::lastContext;

    // get its process
    Process* pro = _current->getProcess();

    // creating a fresh Tid and a new process on it
    u16 newTid = getFreshTid();
    Process* copy = new Process(newTid,getG(pro->getGid()),pro->_fds);

    // Copy RAM mappings.
    //printf("origin RAM Mapping : \n");
    //pro->_usermem.DumpTree();
    copy->_usermem = pro->_usermem; // this step can be very long.
    //printf("new RAM Mapping : \n");
    //copy->_usermem.DumpTree();

    // Creating the new thread and coping the context of the calling thread
    // the new process has only one thread : the copy of the thread that called fork
    Thread* tcopy = new Thread(newTid,0,copy);
    tcopy->context = _current->context;

    // child process has 0 as a return.
    tcopy->context.rax = 0;

    printf("End of fork\n");
    // return to parent process the new pid
    return newTid;
}



void Scheduler::timerHandler(const InterruptParams& params){
    // If no thread is running : we are executing kernel
    if(!_current){
        if(RemainingTime) -- RemainingTime;
        pic.endOfInterrupt(0);
        return; // TODO find a way to tell that in tick has occured
    }

    // If current thread still has time
    if(RemainingTime > 0){
        --RemainingTime;
        pic.endOfInterrupt(0);
        return; // return to current execution
    }

    // saving context
    _current->context = params;

    // thread is now last in queue
    _threadFIFO.push_back((Thread*)_current);
    _current = nullptr;

    // if there is another timer interrupt during run, it will enter first case
    pic.endOfInterrupt(0);
    run();
}






void timerHandler(const InterruptParams& params){
    if((iptr)params.rip <0){
        //printf("Timer Interrupt during kernel execution %p \n",params.rip);
        // TODO store it somewhere
        pic.endOfInterrupt(0);
        return;
    }
    /*int i = 0;
    ++i;
    if((i % 10) != 0){
        pic.endOfInterrupt(0);
        return;
        }*/
    //printf("Timer Interrupt at %p\n",params.rip);
    schedul.timerHandler(params);
}

Scheduler schedul;
