#ifndef SYSCALLS_H
#define SYSCALLS_H

#include"../utility.h"

/**
   @brief This file contains all syscalls in order

   The real implementation may be elsewhere

   In the documentation, a return value of ESOMETHING is be understood as
   following:
   The system call will return the opposite of the error code,
   Then the C handler will return -1 and set errno to this value.
 */

/// Register all syscalls in the handlers table.
void syscallFill();

/**
   @brief Syscall 0, read : read bytes from file descriptor.
 */
u64 sysread(u64 fd, u64 buf, u64 count, u64,u64,u64);

/**
   @brief Syscall 1, write : write bytes to file descriptor.
 */
u64 syswrite(u64 fd, u64 buf, u64 count, u64,u64,u64);

/**
   @brief Syscall 2, open : open a file.
*/
u64 sysopen(u64 path, u64 flags, u64,u64,u64,u64);

/**
   @brief Syscall 3, close : close a file descriptor.
*/
u64 sysclose(u64 fd, u64,u64,u64,u64,u64);

/**
   @brief Syscall 12, brk : move the position of user's brk.
 */
u64 sysbrk(u64 addr, u64,u64,u64,u64,u64);

/**
   @brief Syscall 22, pipe : Create an anonymous pipe.
   @param fd2 (int*) : Table of integer of size 2 : the read end will be in
   fd2[0] and the write end will be in fd[1].
   @retval 0 success
   @retval EFAULT The pointer fd2 is not valid.
*/
u64 syspipe(u64 fd2, u64,u64,u64,u64,u64);

/**
   @brief Syscall 32, dup : Duplicate a file descriptor;
*/
u64 sysdup(u64 oldfd, u64,u64,u64,u64,u64);

/**
   @brief Syscall 33, dup2 : Duplicate a file descriptor;
*/
u64 sysdup2(u64 oldfd, u64 newfd, u64,u64,u64,u64);


/**
   @brief Syscall 56, clone : create a new thread in the current process.
 */
u64 sysclone(u64 rip, u64 rsp, u64,u64,u64,u64);

/**
   @brief Syscall 57, fork : create a new process, a copy from the current one.
 */
u64 sysfork(u64,u64,u64,u64,u64,u64);

/**
   @brief Syscall 58, exit : exit the current process with code rc
 */
u64 sysexit(u64 rc, u64,u64,u64,u64,u64);

/**
   @brief Syscall 57, exec : replace current process by a new one.
 */
u64 sysexec(u64 path, u64 argv, u64,u64,u64,u64);

/**
   @brief Syscall 60, texit : exit the current thread with code rc
 */
u64 systexit(u64 rc, u64,u64,u64,u64,u64);

/**
   @brief Syscall 61, wait : wait for a child process to die.
 */
u64 syswait(u64 pid, u64 status, u64,u64,u64,u64);



#endif
