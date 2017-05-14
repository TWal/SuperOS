#ifndef __SUPOS_ERRNO_H
#define __SUPOS_ERRNO_H

/**
   @brief error number of standard library.

   @todo thread specific version.
 */
extern int errno;

// #define errors numbers
#define EBADF 1
#define ECHILD 2
#define EPIPE 3 ///< When writing on a closed pipe
#define EACCESS 4 ///< When accessing an file that do not exists



#endif
