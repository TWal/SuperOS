#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../utility.h"
#include "../Interrupts/Interrupt.h"
#include "Process.h"
#include <map>
#include <deque>

class Scheduler{
public :
    Scheduler();
    void init(Thread* pro);// load init process
    [[noreturn]] void run(); // run the next program, the previous context should have been saved
    [[noreturn]] void exit(u64 returnCode);
    void timerHandler(const InterruptParams&);
private :
    std::map<int,Process*> _processes;
    std::map<Thread*,bool> _isAlive;
    std::deque<Thread*> _threadFIFO; // no priority queue for now.
    Thread* volatile _current; // nullptr when no thread is active (in kernel mode)
    u8 RemainingTime; // number of tick remaining for _current
};

void timerHandler(const InterruptParams&);

extern Scheduler schedul;


#endif
