#include "Scheduler.h"
#include "../Interrupts/Pic.h"
#include "../Interrupts/Pit.h"
#include <stdio.h>
#include "../User/Syscall.h"

Scheduler::Scheduler() : _current(nullptr),RemainingTime(0){
    Pit::set(0,50000,Pit::SQUAREWAVE); // for now slow interruptions (~ 20 Hz)
    // we will speed up later when its stable.

}
u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}

void Scheduler::init(Thread* initThread){
    idt.addInt(0x20,::timerHandler);
    pic.activate(Pic::TIMER);
    Process* pro = initThread->getProcess();
    _processes[pro->getPid()] = pro;
    _isAlive[initThread] = true;
    _threadFIFO.push_back(initThread);

    //declare scheduler syscalls :
    handlers[SYSEXIT] = sysexit;

}

[[noreturn]] void Scheduler::run(){
    assert(!_current);
    assert(_threadFIFO.size() > 0 && "Empty Scheduler");
    _current = _threadFIFO[0];
    _threadFIFO.pop_front();
    if(_current->wr){
        _threadFIFO.push_back((Thread*)_current);
        _current = nullptr;
        run();
    }

    RemainingTime = 5; // by default program runs for 10 ticks (1 tick is 1 ms)
    _current->run();

}
[[noreturn]] void Scheduler::exit(u64 returnCode){
    assert(_current);
    Process* pro = _current->getProcess();
    pro->terminate(returnCode); // TODO delete thread in scheduler
    if(pro->getPid() == 1){ // if init died
        printf("init died with code %lld",returnCode);
        kend(); // shutdown.
    }
    else{
        run();
    }
}

void Scheduler::timerHandler(const InterruptParams& params){
    breakpoint;
    if(!_current){
        if(RemainingTime) -- RemainingTime;
        pic.endOfInterrupt(0);
        return; // TODO find a way to tell that in tick has occured
    }
    if(RemainingTime > 0){
        --RemainingTime;
        pic.endOfInterrupt(0);
        return; // return to current execution
    }
    _current->storeContext(params);
    _threadFIFO.push_back((Thread*)_current);
    _current = nullptr;
    pic.endOfInterrupt(0); // if there is another timer interrupt during run, it will
    run();
}






void timerHandler(const InterruptParams& params){
    if((iptr)params.rip <0){
        //printf("Timer Interrupt during kernel execution %p \n",params.rip);
        // TODO store it somewhere
        pic.endOfInterrupt(0);
        return;
    }
    //printf("Timer Interrupt at %p\n",params.rip);
    schedul.timerHandler(params);
}

Scheduler schedul;
