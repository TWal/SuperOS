#ifndef __SUPOS_STDARG_H
#define __SUPOS_STDARG_H

#ifdef __cplusplus
extern "C" {
#endif

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l) __builtin_va_arg(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_copy(v1,v2) __builtin_va_copy(v1,v2)
typedef __builtin_va_list va_list;

#ifdef __cplusplus
}
#endif

#endif
