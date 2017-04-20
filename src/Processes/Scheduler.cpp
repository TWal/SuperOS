#include "Scheduler.h"
#include "../Interrupts/Pic.h"
#include "../Interrupts/Pit.h"
#include <stdio.h>
#include "../User/Syscall.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"

void* const tidBitset = (void*)(-0x80000000ll - 0x2000ll);


Scheduler::Scheduler() : _current(nullptr),RemainingTime(0){
    Pit::set(0,10000,Pit::SQUAREWAVE); // for now slow interruptions (~ 20 Hz)
    // we will speed up later when it's stable.

}
static u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}
static u64 sysfork(u64,u64,u64,u64,u64,u64){
    return schedul.fork();
}
static u64 sysclone(u64 rip,u64 rsp,u64,u64,u64,u64){
    return schedul.clone(rip,rsp);
}
static u64 sysbrk(u64 addr,u64,u64,u64,u64,u64){
    printf("sysbrk with %p\n",addr);
    assert((i64)addr >= 0);
    return schedul.brk((void*)addr);
}

void Scheduler::init(Thread* initThread){
    assert(initThread->getTid() == 1);

    //activate timer interrupts
    idt.addInt(0x20,::timerHandler);
    pic.activate(Pic::TIMER);

    //activate tid bitset;
    paging.createMapping(physmemalloc.alloc(),tidBitset);
    _tids.init(tidBitset,0x1000*8);
    _tids.fill();
    _tids[0] = false; // no thread with number 0.
    _tids[1] = false;

    //declare scheduler syscalls :
    handlers[SYSEXIT] = sysexit;
    handlers[SYSFORK] = sysfork;
    handlers[SYSCLONE] = sysclone;
    handlers[SYSBRK] = sysbrk;

}

[[noreturn]] void Scheduler::run(){
    assert(!_current);
    assert(_threadFIFO.size() > 0 && "Empty Scheduler");

    printf("current queue is : ");
    for(auto t : _threadFIFO){
        printf("%d ",t->getTid());
    }//printf("\n");

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
Thread* Scheduler::enterSys(){
    // we must be in a thread
    assert(_current);
    // we save that thread context.
    _current->context = *Context::lastContext;
    Context::lastContext = nullptr;
    return _current;
}
void Scheduler::stopCurent(){
    if(Context::lastContext) enterSys();
    _current = nullptr;
}
u16 Scheduler::fork(){
    Thread* old = enterSys();

    printf("Fork on instruction %p\n",_current->context.rip);

    // get its process
    Process* pro = old->getProcess();

    // creating a fresh Tid.
    u16 newTid = getFreshTid();

    // Copy process onto the new tid.
    new Process(*pro,old,newTid);

    printf("End of fork, created %d\n",newTid);
    // return to parent process the new pid
    return newTid;
}
u16 Scheduler::clone(u64 rip, u64 stack){
    assert(stack && (i64)rip > 0);
    Thread* old = enterSys();
    printf("Clone on instruction %p to instruction %p \n",_current->context.rip,rip);
    Process* pro = old->getProcess();
    u16 newTid = getFreshTid();

    // creating a new thread with all his register UB except rip and rsp.
    Thread* newTh = new Thread(newTid,rip,pro);
    newTh->context.rsp = stack;


    printf("End of clone, created %d\n",newTid);
    return newTid;
}

u64 Scheduler::brk(void* addr){
    Thread* t = enterSys();
    auto tmp = t->getProcess()->_heap.brk(addr);
    printf("brk on %d called with %p return %p\n",t->getTid(),addr,tmp);
    t->getProcess()->_usermem.DumpTree();
    return tmp;
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
