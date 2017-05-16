#ifndef __SUPOS_STDIO_H
#define __SUPOS_STDIO_H


#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)

    struct FILE;
    typedef struct FILE FILE;

    extern FILE* stdin;
    extern FILE* stdout;
    extern FILE* stderr;
    extern FILE* stdlog;

    int fgetc(FILE* file);
    int fputc(int character,FILE* file);
    char* fgets(char* str, int num, FILE* file);
    int fputs(const char* str, FILE* file);
#define getc fgetc
#define putc fputc
    int getchar();
    int putchar(int character);
    // gets is deprecated(C++) / out of standard (C).
    int puts(const char* str);
    int vfprintf (FILE* file, const char * format, va_list ap);
    int fprintf(FILE* file, const char* format, ...);
    int printf(const char* format, ...);

#ifndef SUP_OS_KERNEL
    // file manipulation
    FILE* fopen(const char* pathname, const char* mode);
    int fclose(FILE* stream);
    FILE* fdopen(int fd, const char* mode);
    int fseek(FILE* stream, long int offset, int origin);
    long int ftell(FILE* stream);
    void rewind(FILE* stream);
#endif

    size_t fread(void* buf, size_t size, size_t count, FILE* stream);
    size_t fwrite(void* buf, size_t size, size_t count, FILE* stream);


#ifdef __cplusplus
}
#endif


#endif
