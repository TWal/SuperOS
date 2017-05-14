#ifndef __SUPOS_UNISTD_H
#define __SUPOS_UNISTD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    enum{O_RDONLY = 1, O_WRONLY = 2, O_RDWR = 3, O_CREAT = 4, O_TRUNC = 8, O_APPEND = 16};

    typedef unsigned short pid_t;

    size_t read(int fd, void* buf, size_t count);
    size_t write(int fd, const void* buf, size_t count);

    int open(const char* path, int flags);
    int close(int fd);

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
