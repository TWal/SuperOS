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
    u32 getFreeFD(); ///< Return a free file descriptor
};








/**
   @page proc Process System

   The process system is the part of the operating system that permit
   to have application to run on it.

   The is different component Process, ProcessGroup and Thread.

   Basically :
       - A Process is an application running to do something
       - A ProcessGroup is a grouping of processes doing something together
       - A Thread is one running execution (instructions, registers, ...)

   Internal protocols are defined in @subpage proc_prot.

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

/**
   @page proc_prot Process protocols

   @brief This page describe in which order function must be called in kernel
   for Process/Thread creation/destruction.

   For Creation, the constructor suffice.

   @section prot_ties Reference between objects

   At the destruction all the created links must be desctructed in the right order.

   A Thread is referenced from :
       - @ref Process::_threads of its owner process (@ref Thread::_process).
       - @ref Scheduler::_threads in @ref schedul.
       -  it may be referenced from @ref _current

   A Process is referenced from :
       - @ref Thread::_process in all its thread.
       - @ref Scheduler::_processes in @ref schedul.
       - @ref Process::_sons in its parent.
       - @ref Process::_parent in all its sons.
       - @ref ProcessGroup::_processes it its group.

   A Process group is only referenced by the scheduler.

   A processes can have three state, active, zombie (The Process is still member of
   the Process heirarchy but own no ressources), destroy (The class Process has been destroyed).

   A processes may not own any thread when zombie (and furthermore when destroyed).
   The thread class may be still in Heap but near to destruction.


   @section prot_del Deletion

   When a thread dies (texit):
       - It is removed from @ref _current
       - A call to @ref Thread::terminate must be issued : the thread is
         unregistered from its owner.
       - The Thread object is then destroyed (which include removing it from
         scheduler)

   When a process exits (exit):
       - Any remaining thread of it are removed from _current.
       - @ref Process::terminate is called : all resources of
       the processes are freed :
           - Its @ref Process::_usermem "memory".
           - The destructor of all Threads is called thus freing them from
             @ref schedul. The @ref Process::_threads is cleared
           - All the file descriptors if @ref Process::_fds are closed.
       - The process is now in zombie state.

   When a process is destroyed (its parent waited him):
       - The destructor is simply called : it is removed from @ref schedul,
         all its sons are @ref Process::orphan "orphaned" and it is removed
         from its @ref ProcessGroup group.

   @warning Its not removed from the @ref Process::_sons because normaly
   it is the parent that delete the son.

   A ProcessGroup is automatically destroyed and removed from @ref schedul when
   all its member are destroyed.



 */


#endif
