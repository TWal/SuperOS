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
#include "Waiting.h"

#define FIXED_SIZE_STACK 1

class Thread;
class Process;


/**
   @brief Represent a process group, with gid @reg _gid.

   The leader of the group is the process that has his pid equal to @ref _gid

   One can send a signal to a process or a process group. cf kill and killpg

 */

class ProcessGroup{
    u16 _gid;
public :
    u16 _uid;
private:
    /// List of members processes
    std::set<Process*> _processes;
public :
    explicit ProcessGroup(u16 gid);
    ~ProcessGroup();
    /// Add a process
    void addProcess(Process* pro){_processes.insert(pro);}
    /// Remove a process
    void remProcess(Process* pro);
    /// Get the gid of the group
    u32 getGid(){return _gid;}
};

/**
   @brief Represent a process with pid @ref _pid.

   A process is a component that represent a bunch of
   Threads running together and sharing :
      - A common memory mapping (@ref _usermem)
      - A Heap (@ref _heap)
      - A file descriptor table (and thus all opened file and windows)

   A Process is member of a group whose gid is @reg _gid.
   It also has any number of Thread (at least one : a process dies if it has
   no Threads).



 */
class Process : public Waitable{
    friend Thread;
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
public :
    /// The heap of the process.
    Heap _heap;
    /// The memory of the process
    UserMemory _usermem;
    /// The list of file descriptors of the process.
    std::vector<FileDescriptor> _fds;
    /// Only init will be built by this constructor.
    Process(u32 pid,ProcessGroup* pg,
            std::vector<FileDescriptor> fds = std::vector<FileDescriptor>());
    Process(Process& other,Thread* toth, u16 pid); ///< fork;
    ~Process();
    /**
       @brief Load the elf64 file Bytes and create the main thread starting
       on its entry point.

       Implementation of exec. Deletes all threads, clear heap and memory, then
       launch a new process from the parameter file.
    */
    Thread* loadFromBytes(Bytes* file);
    void clear(); ///< clear the process : no thread, clean mapped memory
    u32 getPid()const {return _pid;}
    u32 getGid()const {return _gid;}
    void addThread(Thread* thread);
    /**
       @brief Terminates the process, it is now a zombie.

       The returnCode override the main return code
     */
    void terminate(u64 returnCode);
    /**
       @brief Terminates the main threads

     */
    void mainTerm(Thread* main,u64 returnCode);
    /// Closes a thread
    void remThread(Thread* thread);
    /// Prepare for execution (enable process memory in the paging system)
    void prepare();
    bool isLeader()const{return _pid == _gid;}
    /// Add a child
    void addChild(Process* pro);
    void orphan(); ///< Call to orphan a process (become init child).
};

/**
   @brief Represents a thread of execution

   A thread is member of a process (@ref _process) but have its own processor
   state in  @ref context.

 */

class Thread : public Waiting {
    friend Process;
    u16 _tid;
    u16 _pid; /// Must be equal to _process->getPid();
    u16 _gid;
public :
    u16 _uid;
private :
    Process* _process; /// Process of the thread, redundant with _pid for speed.
public :
    Context context;
    Thread(u16 tid,u64 rip,Process* process);
    ~Thread();
    [[noreturn]] void run(); /// Launch the thread until the next timer interruption.
    u16 getTid()const{return _tid;}
    u16 getPid()const{return _pid;}
    u16 getGid()const{return _gid;}
    bool isMain()const{return _tid == _pid;}
    /// Close the thread, returnCode is ignored if this thread is not the main thread.
    void terminate(u64 returnCode);
    Process* getProcess(){return _process;}
    /// Execute the wait system call on any child process.
    u64 waitp(u64* status);
    /// Execute the wait system call on the given child process.
    u64 waitp(Process* pro, u64* status);
};

/**
   @brief Initialize process system

   Currently just setup system call like read, write, brk , ...
*/
void ProcessInit();


/**
   @page proc Process System

   The process system is the part of the operating system that permit
   to have application to run on it.

   The is different component Process, ProcessGroup and Thread.

   Basically :
       - A Process is an application running to do something
       - A ProcessGroup is a grouping of processes doing something together
       - A Thread is one running execution (instructions, registers, ...)

   @section proc_proc Processes

   A Process is Application, a blocks of machinery doing
   something useful.

   A Process is member of the process hierarchy : a OS-wide tree of all processes.
   The root process is init with PID 1. All other are children, grand-children, ...
   of init.

   A Process can create an other process with fork. The child process can now lives
   its life, but when it dies (@ref Process::terminated), he does not disappear
   from the hierarchy, it is a zombie: he lives until the parent wait to get the
   pid of one of its dead child.

   If the parent dies first, the child is @ref Process::orphan "orphaned",
   It becomes a child of init.

   When a process dies, it need a @ref Process::_returnCode "return code".
   If it has died because of exit call, his return code id the parameter of
   th system call.
   It it has died because of lack of remaining thread, its return code is
   the one of its main thread.

   @section proc_pg Process Groups

   Every process is member of a process group, for example all the foreground
   processes of a shell.

   The main purpose of a group is to send signals to all member processes at the same time.

   @section proc_th Threads

   Threads are a running instance of a process, a linear piece of control flow.
   The threads all run on the same memory of their process but have their own
   register and processor state.

   The threads are run one after another by the Scheduler.

   The Thread which have a TID equal to the PID of his process, is the main thread.
   If a process dies, because all its thread are dead, its return code is the one
   of the main thread.

   The return code of other thread is only there for debugging purposes.


 */



#endif
