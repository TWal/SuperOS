#ifndef __SUPOS_STDIO_H
#define __SUPOS_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUP_OS_KERNEL
void printf(const char* s, ...);
#endif

#ifdef __cplusplus
}
#endif


#endif
