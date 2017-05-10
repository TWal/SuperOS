#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../utility.h"
#include "../Interrupts/Interrupt.h"
#include "Process.h"
#include <map>
#include <deque>

/**
   @brief This class Handles the scheduling of the different user programs.

   This class store the mapping from id for user object to their representation
   in kernel(cf. Thread, Process, ProcessGroup).


 */
class Scheduler{
public :
    Scheduler();
    void init(Thread* pro);// load init process
    [[noreturn]] void run(); // run the next program, the previous context should have been saved
    [[noreturn]] void exit(u64 returnCode);
    [[noreturn]] void texit(u64 returnCode);
    Thread* enterSys();
    void stopCurent();
    u16 fork(); ///< fork current process;
    /// Creates a Thread stating at rip with stack rsp.
    u16 clone(u64 rip, u64 stack);
    u16 wait(i64 pid, int* status);
    void timerHandler(const InterruptParams&);
    Thread* getT(u16 tid){return _threads.at(tid);}
    Process* getP(u16 pid){return _processes.at(pid);}
    ProcessGroup* getG(u16 gid){return _groups.at(gid);}
    void addT(u16 tid,Thread* th){_threads[tid]=th; _threadFIFO.push_back(th);}
    void addP(u16 pid,Process* pro){_processes[pid]=pro;}
    void addG(u16 gid,ProcessGroup* pg){_groups[gid]=pg;}
    void freeT(u16 tid){
        assert(_threads.count(tid));
        _threads.erase(tid);
        if(!_groups.count(tid)&&!_processes.count(tid)){
            _tids.set(tid);
        }
    }
    void freeP(u16 pid){
        assert(_processes.count(pid));
        _processes.erase(pid);
        if(!_groups.count(pid)){
            _tids.set(pid);
        }
    }
    void freeG(u16 gid){
        assert(_groups.count(gid));
        _groups.erase(gid);
        _tids.set(gid);
    }
private :
    std::map<u16,Process*> _processes;
    std::map<u16,Thread*> _threads;
    std::map<u16,ProcessGroup*> _groups;
    std::deque<Thread*> _threadFIFO; // no priority queue for now.
    Thread* volatile _current; // nullptr when no thread is active (in kernel mode)

    static const u8 processQuantum = 5; ///< Number of tick each thread gets when starting.
    volatile u8 _remainingTime; ///< Number of tick remaining for _current
    static const u8 renderingQuantum = 10; ///< Number of tick between renderings
    volatile u8 _timeToRendering; ///< Number of tick remaining before rendering
    volatile bool _halted;
    u64 _runTryNum;
    Bitset _tids; // used tid : 1 is free, 0 is occupied
    inline u16 getFreshTid(){
        u16 res = _tids.bsf();
        _tids.unset(res);
        return res;
    }
};

void timerHandler(const InterruptParams&);

extern Scheduler schedul;


#endif
