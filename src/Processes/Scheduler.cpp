#include "Scheduler.h"
#include "../Interrupts/Pic.h"
#include "../Interrupts/Pit.h"
#include <stdio.h>
#include "../User/Syscall.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"
#include "../log.h"

void* const tidBitset = (void*)(-0x80000000ll - 0x2000ll);


Scheduler::Scheduler() : _current(nullptr),_remainingTime(0),
                         _halted(false),_timeToRendering(0),_runTryNum(0){
    Pit::set(0,1000,Pit::SQUAREWAVE); // for now slow interruptions (~ 20 Hz)
    // we will speed up later when it's stable.
}
static u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}
static u64 systexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.texit(rc);
}
static u64 sysfork(u64,u64,u64,u64,u64,u64){
    return schedul.fork();
}
static u64 sysclone(u64 rip,u64 rsp,u64,u64,u64,u64){
    return schedul.clone(rip,rsp);
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
    handlers[SYSTEXIT] = systexit;
    handlers[SYSFORK] = sysfork;
    handlers[SYSCLONE] = sysclone;

}

[[noreturn]] void Scheduler::run(){
    assert(!_current);
    assert(_threadFIFO.size() > 0 && "Empty Scheduler");


    debug(Schedul,"current queue is : ");
    for(auto tid : _threadFIFO){
        debug(Schedul,"%d ",tid);
    }//printf("\n");
    if(_runTryNum >= _threadFIFO.size()){
        // If all processes are waiting
        debug(Schedul,"Idle\n");
        _runTryNum = 0;
        cli;
        _halted = true;
        // passive sleep : processor stopped until next interruption.
        // TODO interrupts may return from this
        while(true) asm volatile("xor %rsp,%rsp; sti; hlt");
        error(Schedul," After while (true) hlt : Should never happen");
    }

    // pop the next candidate's TID
    u16 nextTid = _threadFIFO[0];
    _threadFIFO.pop_front();
    auto it  = _threads.find(nextTid);
    // if it has been deleted continue
    if(it == _threads.end()) run();

    _current = it->second;

    // If this thread is waiting something
    if(!_current->OK()){
        _threadFIFO.push_back(nextTid);
        _current = nullptr;
        ++_runTryNum;
        run();
    }

    // All is right, this thread can run
    _remainingTime = processQuantum; // by default program runs for processQuantum ticks
    _runTryNum = 0; // OK we have found it
    _current->run();

}






[[noreturn]] void Scheduler::exit(u64 returnCode){
    assert(_current);
    Process* pro = _current->getProcess();
    u16 tid = _current->getTid();
    _current = nullptr;
    info(Schedul,"Process %d died because thread %d exited with %lld\n",
         pro->getPid(),tid,returnCode);
    pro->terminate(returnCode); // call kend if it is init.

    run();
}

[[noreturn]] void Scheduler::texit(u64 returnCode){
    assert(_current);
    Thread* th = _current;
    u16 tid = _current->getTid();
    _current = nullptr;
    info(Schedul,"Thread %d died with %lld\n",tid,returnCode);
    th->terminate(returnCode);

    delete th;
    run();
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
    _threadFIFO.push_back(_current->getTid());
    _current = nullptr;
}





u16 Scheduler::fork(){
    Thread* old = enterSys();

    info(Schedul,"Fork by %d, on instruction %p\n",_current->getTid(),_current->context.rip);

    // get its process
    Process* pro = old->getProcess();

    // creating a fresh Tid.
    u16 newTid = getFreshTid();

    // Copy process onto the new tid.
    new Process(*pro,old,newTid);

    debug(Schedul,"End of fork, created %d\n",newTid);
    // return to parent process the new pid
    return newTid;
}


u16 Scheduler::clone(u64 rip, u64 stack){
    assert(stack && (i64)rip > 0);
    Thread* old = enterSys();
    info(Schedul,"Clone on instruction %p to instruction %p \n",_current->context.rip,rip);
    Process* pro = old->getProcess();
    u16 newTid = getFreshTid();

    // creating a new thread with all his register UB except rip and rsp.
    Thread* newTh = new Thread(newTid,rip,pro);
    newTh->context.rsp = stack;


    debug(Schedul,"End of clone, created %d\n",newTid);
    return newTid;
}






void Scheduler::timerHandler(const InterruptParams& params){
    if(_remainingTime > 0){
        --_remainingTime;
    }
    if(_timeToRendering > 0){
        --_timeToRendering;
    }
    if(_halted){
        assert((iptr)params.rip < 0);
        if(params.rsp != 0) warning(Schedul, "Halted rsp not 0");
        _halted = false;
        pic.endOfInterrupt(0);
        if(_timeToRendering == 0){
            kloop();
            _timeToRendering = renderingQuantum;
        }
        run();
    }
    else{
        if((iptr)params.rip < 0){
            pic.endOfInterrupt(0);
            return;
        }
        debug(Schedul,"Time Interrupt at %p",params.rip);
        Context::save(params); // saving context.
        pic.endOfInterrupt(0);
        // prempting userMode program.
        if(_timeToRendering == 0){
            stopCurent();
            //debug("Rendering");
            kloop();
            _timeToRendering = renderingQuantum;
            run();
        }
        if(_remainingTime == 0){
            stopCurent();
            run();
        }
        // nothing to do
    }


    pic.endOfInterrupt(0);
    return;
}






void timerHandler(const InterruptParams& params){
    schedul.timerHandler(params);
}



Scheduler schedul;
