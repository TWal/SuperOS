#ifndef SYSCALL_H
#define SYSCALL_H

#include"../utility.h"

/**
   @file This file define the syscall API and handlers registering

   It also defines all syscall numbers.
   All syscalls that have an equivalent in linux have the same number.
*/


/// Numbers of different syscalls allowed.
#define SYSCALLNUM 256

#define SYSERROR (-1) ///< Called if no handler is activated
#define SYSREAD 0
#define SYSWRITE 1
#define SYSOPEN 2
#define SYSCLOSE 3
#define SYSSTAT 4
#define SYSFSTAT 5
#define SYSLSTAT 6
#define SYSPOLL 7

#define SYSSEEK 8
#define SYSBRK 12

#define SYSPIPE 22

#define SYSOPENWIN 24
#define SYSOPENTWIN 25
#define SYSRESIZEWIN 26

#define SYSDUP 32
#define SYSDUP2 33

#define SYSGETGID 36
#define SYSGETTID 37
#define SYSGETPPID 38
#define SYSGETPID 39

#define SYSTEST 42

#define SYSCLONE 56 ///< Clone is only for thread: clone(new rip, new rsp).
#define SYSFORK 57 ///< Fork t create a new process.
#define SYSEXIT 58 // vfork is useless and 231 for exit_group is too far away
#define SYSEXEC 59 ///< launch another program.
#define SYSTEXIT 60
#define SYSWAIT 61
#define SYSKILL 62


#define SYSOPENDIR 72
#define SYSREADDIR 73
#define SYSSEEKDIR 74
#define SYSTELLDIR 75
#define SYSGETCWD 79

#define SYSCHDIR 80
#define SYSFCHDIR 81
#define SYSRENAME 82
#define SYSMKDIR 83
#define SYSRMDIR 84
#define SYSLINK 86
#define SYSUNLINK 87

#define SYSFUTEX 202


void syscallInit();

/// @brief The type of a system call handler
/// @details The real handler is free to ignore some value.
using syscallHandler = u64(*)(u64,u64,u64,u64,u64,u64);


/**
   @brief Contains the table of system calls

   If a pointer is null, this syscall is disabled,
   else the pointer a valid function of type @ref syscallHandler.

 */
extern "C" syscallHandler * const handlers;

/**
   @page page_sys System Calls

   @brief Description of system call management.

   @section sys_use Syscall usage

   The syscall system handle syscall/sysret instructions but also int 0x80.

   Each syscall has a number and take up to six argument. To enable a syscall,
   just put the pointer to a function of type @ref syscallHandler in
   @ref handlers[num of syscall]. To disable it just put nullptr.

   When the user call a disable syscall (or a number outside of [0,@ref SYSCALLNUM[ ),
   the syscall -1 is called, This is the function @ref syserror that just does
   a @ref bsof "Blue Screen".

   The syscall handler can assume that the global var Context::lastContext is
   correctly set on entry.

   @section sys_conv Syscall calling convention

   We just take the convention of linux i.e :

   @subsection sys_scinstr Syscall instruction

   The number of the system call goes in rax, the 6 arguments are given in this
   order :
       - rdi
       - rsi
       - rdx
       - r10
       - r8
       - r9

   The return value is in rax.
   All registers are preserved except rax (for return value), rcx and r11.

   @subsection sys_int80 Interrupt 0x80.

   The number of the system call goes in rax, the 6 arguments are given in this
   order :
   - rbx
   - rcx
   - rdx
   - rsi
   - rdi
   - rbp

   The return value is in rax.
   All registers are preserved except rax (for return value).

   This version is theoretically slightly slower than the other.

   @section sys_list Syscall list

   The list of the currently supported System calls can be found here :
   @ref Syscalls.h

 */


#endif
