#ifndef __SUPOS_UNISTD_H
#define __SUPOS_UNISTD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned short pid_t;

    size_t read(int fd, void* buf, size_t count);
    size_t write(int fd, const void* buf, size_t count);

    pid_t fork();

#ifdef __cplusplus
}
#endif

#endif
