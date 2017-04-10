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
    Thread* getT(u16 tid){return _threads.at(tid);}
    Process* getP(u16 pid){return _processes.at(pid);}
    ProcessGroup* getG(u16 gid){return _groups.at(gid);}
private :
    std::map<u16,Process*> _processes;
    std::map<u16,Thread*> _threads;
    std::map<u16,ProcessGroup*> _groups;
    std::deque<Thread*> _threadFIFO; // no priority queue for now.
    Thread* volatile _current; // nullptr when no thread is active (in kernel mode)
    u8 RemainingTime; // number of tick remaining for _current
    Bitset _tids; // used tid : 1 is free, 0 is occupied
};

void timerHandler(const InterruptParams&);

extern Scheduler schedul;


#endif
