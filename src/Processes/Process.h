#ifndef PROCESS_H
#define PROCESS_H

#include "../utility.h"
#include "../Memory/Paging.h"
#include <vector>
#include <set>
#include "FileDescriptor.h"
#include "../Bytes.h"
#include "../Memory/Heap.h"
#include "../Memory/UserMemory.h"
#include "../Interrupts/Interrupt.h"
#include "../User/Context.h"

#define FIXED_SIZE_STACK 1

class Thread;
class Process;

class ProcessGroup{
    u16 _gid;
public :
    u16 _uid;
private:
    std::set<Process*> _processes;
public :
    explicit ProcessGroup(u16 gid);
    void addProcess(Process* pro){_processes.insert(pro);}
    u32 getGid(){return _gid;}
};

class Process{
    u16 _pid;
    u16 _gid;
public :
    u16 _uid;
private:
    std::set<Thread*> _threads;
    Process * _parent;
    std::set<Process *> _sons;
    bool _terminated; // process is a zombie and can be waited.
    bool _mainTerminated; // main thread has terminated but others may not.
    u64 _returnCode;
    //Heap* heap;
public :
    UserMemory _usermem;
    std::vector<FileDescriptor*> _fds;
    Process(u32 pid,ProcessGroup* pg,
            std::vector<FileDescriptor*> fds = std::vector<FileDescriptor*>());
    ~Process();
    // load the elf64 file Bytes and create the main thread starting on its entry point.
    Thread* loadFromBytes(Bytes* file);
    void clear(); // clear the process : no thread, clean mapped memory
    u32 getPid(){return _pid;}
    u32 getGid(){return _gid;}
    void addThread(Thread* thread);
    void terminate(u64 returnCode);
    void mainTerm(Thread* main,u64 returnCode);
    void remThread(Thread* thread);
    void prepare();
    bool isLeader(){return _pid == _gid;}
    //void orphan(); // call to orphan a process (becom init child).
};

class WaitingReason; // TODO implement that

class Thread{
    u16 _tid;
    u16 _pid; // must be equal to _process->getPid();
    u16 _gid;
public :
    u16 _uid;
private :
    Process* _process; // process of the thread, redundant with _pid for speed
public :
    Context context;
    Thread(u16 tid,u64 rip,Process* process);
    ~Thread();
    WaitingReason* wr; // if wr == nullptr, the thread is runnable.
    [[noreturn]] void run(); // launch the thread until the next timer interruption
    u16 getTid(){return _tid;}
    u16 getPid(){return _pid;}
    u16 getGid(){return _gid;}
    bool isMain(){return _tid == _pid;}
    // close the thread, returnCode is ignored if this thread is not the main thread
    void terminate(u64 returnCode);
    Process* getProcess(){return _process;}
};

#endif
