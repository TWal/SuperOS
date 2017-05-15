#include <stdarg.h>
#include "../stdio.h"
#include "../unistd.h"
#include "../string.h"

int errno;
int* __errno_location = &errno;

struct FILE{
    int fd;
};

static FILE _stdin = {0};
FILE* stdin = &_stdin;
static FILE _stdout = {1};
FILE* stdout = &_stdout;
static FILE _stderr = {2};
FILE* stderr = &_stderr;
static FILE _stdlog = {3};
FILE* stdlog = &_stdlog;



int fgetc(FILE* file){ // TODO MT-safe
    char ret;
    int res = read(file->fd,&ret,1);
    if (res == 0 || res == -1) return EOF;
    else return ret;
}

int fputc(int character,FILE* file){
    int res = write(file->fd,&character,1);
    if (res == 0 || res == -1) return EOF;
    else return character;
}

char* fgets (char* str, int num, FILE* file){// TODO bufferize
    char* str2 = str;
    while(num > 1){
        char c = fgetc(file);
        if(c == EOF) break;
        if(c == '\n'){
            *str2 = c;
            ++str2;
            --num;
            break;
        }
        *str2 = c;
        ++str2;
        --num;
    }
    if(str2 == str) return NULL;
    *str2 = 0;
    return str;
}

int fputs(const char* str, FILE* file){
    int num = strlen(str);
    while (num >0){
        int res = write(file->fd,str,num);
        if(res == 0 || res == -1) return EOF;
        num -= res;
        str += res;
    }
    return num;
}

int getchar(){
    return fgetc(stdin);
}
int putchar(int character){
    return fputc(character,stdout);
}

int puts(const char* str){
    if(fputs(str,stdout) == EOF) return EOF;
    return putchar('\n');
}

// vfprintf ....

// return the printed character on success, EOF on failure.
int printDigit(int d, FILE* file){
    if(d <= 9) {
        return putc('0' + d,file);
    } else {
        return putc('a' + (d-10),file);
    }
}

// return the printed size on success, EOF on failure
typedef unsigned long long int ulint;
typedef long long int lint;
typedef unsigned int uint;
int printUInt(ulint n, uint base, uint padding, FILE* file){
    ulint i = 1;
    uint nb = 0;
    while(n/i >= base) {
        i *= base;
        if(padding > 0) {
            padding -= 1;
        }
    }

    for(uint j = 1; j < padding; ++j) {
        if(putc('0', file) == EOF) return EOF;
        ++nb;
    }

    while(n > 0) {
        if(printDigit(n/i,file) == EOF) return EOF;
        n %= i;
        i /= base;
        ++nb;
    }
    while(i >= 1) {
        if(putc('0', file) == EOF) return EOF;
        i /= base;
        ++nb;
    }
    return nb;
}
int printInt(lint n, uint base, uint padding, FILE* file) {
    int l = 0;
    if(n < 0) {
        putc('-', file);
        n = -n;
        l = 1;
    }
    int res = printUInt(n,base,padding,file);
    if( res == EOF) return EOF;
    return res + l;
}

#define SWITCHSIZE(type,typel,typell) typell val = 0;\
    switch(length){                                  \
        case 0:                                      \
            val = va_arg(ap, type);                  \
            break;                                   \
        case 1 :                                     \
            val = va_arg(ap, typel);                 \
            break;                                   \
        case 2 :                                     \
            val = va_arg(ap, typell);                \
            break;                                   \
    }

int vfprintf (FILE* file, const char * s, va_list ap){
    int nb = 0;
    while(*s) {
        if(*s != '%') {
            if(putc(*s, file) == EOF) return EOF;
            ++nb;
            ++s;
            continue;
        }
        ++s;
        int padding = 0;
        while (*s >= '0' && *s <= '9'){
            padding = 10*padding + (*s - '0');
            ++s;
        }
        int length = 0; // int = 0, long = 1, long long = 2
        if(*s == 'l'){
            ++length;
            ++s;
        }
        if(*s == 'l'){
            ++length;
            ++s;
        }if(*s == '%') {
            if(putc('%', file) == EOF) return EOF;
            ++s;
            ++nb;
        } else if(*s == 'u') {
            SWITCHSIZE(uint,unsigned long int, ulint);
            int res = printUInt(val,10,padding,file);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'd') {
            SWITCHSIZE(int,long int, lint);
            int res = printInt(val,10,padding,file);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'x') {
            SWITCHSIZE(uint,unsigned long int, ulint);
            int res = printUInt(val,16,padding,file);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'p') {
            int res = printUInt(va_arg(ap, uintptr_t), 16, padding, file);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 's') {
            int res = fputs(va_arg(ap, char*), file );
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'c') {
            //integer types smaller than int are promoted to int
            //when used in a ...
            char c = va_arg(ap, int);
            if(fputc(c, file)== EOF) return EOF;
            ++nb;
            ++s;
        } else {
            //omg me dunno wat to do!!!
        }
    }
    return nb;
}

// vsprintf
int fprintf(FILE* file, const char* format, ...){
    va_list ap;
    va_start(ap, format);
    int i = vfprintf(file,format, ap);
    va_end(ap);
    return i;
}

int printf(const char*format,...){
    va_list ap;
    va_start(ap, format);
    int i = vfprintf(stdout,format, ap);
    va_end(ap);
    return i;
}

size_t fread(void* buf, size_t size, size_t count, FILE* stream) {
    size_t res = 0;
    size_t toRead = size*count;
    size_t cur;
    while(toRead > 0 && (cur = read(stream->fd, buf, toRead)) != 0) {
        res += cur;
        toRead -= cur;
    }
    return res/size;
}

FILE* fdopen(int fd, const char* mode){
    FILE* f = malloc(sizeof(FILE));
    f->fd = fd;
    return f;
}

