#include <set>
#include <errno.h>
#include "Thread.h"
#include "Scheduler.h"
#include "Waiting.h"
#include <string.h>


using namespace std;

/* _____ _                        _
  |_   _| |__  _ __ ___  __ _  __| |
    | | | '_ \| '__/ _ \/ _` |/ _` |
    | | | | | | | |  __/ (_| | (_| |
    |_| |_| |_|_|  \___|\__,_|\__,_|
*/
Thread::Thread(u16 tid, u64 rip,Process* process)
    : _tid(tid), _pid(process->getPid()), _gid(process->getGid()),_process(process){
    info(Proc,"Creating thread %d at %p",tid,this);
    schedul.addT(_tid,this);
    process->addThread(this);
    context.rip = rip;
    context.rflags = 2 | (1 << 9);
}
Thread::~Thread(){
    info(Proc,"Deletion of thread %d",_tid);
    schedul.freeT(_tid);
    info(Proc,"Deleted of thread %d",_tid);
    //("thread %d deleted\n",_tid);
}

[[noreturn]] void Thread::run(){
    debug(Proc,"Starting thread %d in %d at %p",_tid,_pid,context.rip);
    _process->prepare();
    //fprintf(stderr,"started\n");
    context.launch();
}
void Thread::terminate(u64 returnCode){
    if(isMain())_process->mainTerm(this,returnCode);
    else _process->remThread(this);
}

u64 Thread::waitp(u64* status){
    debug(Proc,"Thread %p is waiting",this);
    if(_process->_sons.empty()){
        return -ECHILD;
    }
    for(auto pro : _process->_sons){
        if(pro->_terminated){
            u64 res = pro->getPid();
            // TODO check valid address
            if(status) *status = pro->_returnCode;
            delete pro;
            return res;
        }
    }
    u16 tid = _tid;
    wait({_process},[status,tid](Waiting* th,Waitable*){
            debug(Proc,"Checker for waitp in %d",tid);
            th->accept();
            static_cast<Thread*>(th)->getProcess()->prepare();
            static_cast<Thread*>(th)->waitp(status);
            });

    // we are no longer runnable
    schedul.stopCurent();
    // restart the scheduler
    schedul.run();
}

u64 Thread::waitp(Process* pro,u64* status){
    if(!_process->_sons.count(pro)) return -ECHILD;
    if(pro->_terminated){
        u64 res = pro->getPid();
        if(status)*status = pro->_returnCode;
        delete pro;
        return res;
    }
    else{
        // we switch to waiting mode
        wait({_process},[status,pro](Waiting* th,Waitable*){
                th->accept();
                static_cast<Thread*>(th)->getProcess()->prepare();
                static_cast<Thread*>(th)->waitp(pro,status);
                    });

        // we are no longer runnable
        schedul.stopCurent();
        // restart the scheduler
        schedul.run();
    }
}

void* Thread::push(const void* data, size_t size){
    context.rsp -= size;
    assert(_process->_usermem.in((void*)context.rsp));
    memcpy((void*)context.rsp,data,size);
    return (void*)context.rsp;
}
