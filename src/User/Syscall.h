#ifndef SYSCALL_H
#define SYSCALL_H

#include"../utility.h"

#define SYSCALLNUM 256

#define SYSERROR (-1) // if no handler is activated
#define SYSREAD 0
#define SYSWRITE 1
#define SYSOPEN 2
#define SYSCLOSE 3
#define SYSSTAT 4
#define SYSFSTAT 5
#define SYSLSTAT 6
#define SYSPOLL 7 // not sure if we implement this

#define SYSTEST 42

#define SYSFORK 57
#define SYSPEXIT 58 // vfork is useless and 231 for exit_group is too far away
#define SYSEXIT 60

void syscallInit();

using syscallHandler = u64(*)(u64,u64,u64,u64,u64,u64);



extern "C" syscallHandler * const handlers;


#endif
