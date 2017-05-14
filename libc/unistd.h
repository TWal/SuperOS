#ifndef __SUPOS_UNISTD_H
#define __SUPOS_UNISTD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned short pid_t;

    size_t read(int fd, void* buf, size_t count);
    size_t write(int fd, const void* buf, size_t count);

    int dup(int oldfd);
    int dup2(int oldfd, int newfd);

    pid_t fork();
    pid_t clone(void(*func)(void),void* stackEnd);
    pid_t waitpid(pid_t p, int* status);
    pid_t wait(int* status);

    void _exit(int status);
    void _texit(int status);

#ifdef __cplusplus
}
#endif

#endif
