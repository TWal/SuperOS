#include "log.h"
#include "Graphics.h"
#include "../src/IO/Serial.h"

char logBuffer[LOADERBUFFER * 0x1000] __attribute__((section(".data.pages")));

size_t posInLogBuffer = 0;

#define EOF (-1)

int putc(int character,bool log){
    ser.write(character);
    if(!log) return 1;
    if(posInLogBuffer >= LOADERBUFFER * 0x1000){
        ser.write("\n\n Out of logging buffer\n");
        rfail(OutOfLoggingBuffer);
    }
    logBuffer[posInLogBuffer] = character;
    ++posInLogBuffer;
    return 1;
}


int puts(const char* str,bool log){
    while(*str){
        putc(*str,log);
        ++str;
    }
    return 1;
}

// vfprintf ....

// return the printed character on success, EOF on failure.
int printDigit(int d,bool log){
    if(d <= 9) {
        return putc('0' + d,log);
    } else {
        return putc('a' + (d-10),log);
    }
}

// return the printed size on success, EOF on failure
typedef unsigned long long int ulint;
typedef long long int lint;
typedef unsigned int uint;
int printUInt(ulint n, uint base, uint padding,bool log){
    ulint i = 1;
    uint nb = 0;
    while(n/i >= base) {
        i *= base;
        if(padding > 0) {
            padding -= 1;
        }
    }

    for(uint j = 1; j < padding; ++j) {
        if(putc('0',log) == EOF) return EOF;
        ++nb;
    }

    while(n > 0) {
        if(printDigit(n/i,log) == EOF) return EOF;
        n %= i;
        i /= base;
        ++nb;
    }
    while(i >= 1) {
        if(putc('0',log) == EOF) return EOF;
        i /= base;
        ++nb;
    }
    return nb;
}
int printInt(lint n, uint base, uint padding,bool log) {
    int l = 0;
    if(n < 0) {
        putc('-',log);
        n = -n;
        l = 1;
    }
    int res = printUInt(n,base,padding,log);
    if( res == EOF) return EOF;
    return res + l;
}

#define SWITCHSIZE(type,typel,typell) typell val;   \
    switch(length){                                 \
        case 0:                                     \
            val = va_arg(ap, type);                 \
            break;                                  \
        case 1 :                                    \
            val = va_arg(ap, typel);                \
            break;                                  \
        case 2 :                                    \
            val = va_arg(ap, typell);               \
            break;                                  \
    }

int vprintf (bool log,const char * s, va_list ap){
    int nb = 0;
    while(*s) {
        if(*s != '%') {
            if(putc(*s,log) == EOF) return EOF;
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
            if(putc('%',log) == EOF) return EOF;
            ++s;
            ++nb;
        } else if(*s == 'u') {
            SWITCHSIZE(uint,unsigned long int, ulint);
            int res = printUInt(val,10,padding,log);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'd') {
            SWITCHSIZE(int,long int, lint);
            int res = printInt(val,10,padding,log);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'x') {
            SWITCHSIZE(uint,unsigned long int, ulint);
            int res = printUInt(val,16,padding,log);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'p') {
            int res = printUInt(va_arg(ap, uintptr_t), 16, padding,log);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 's') {
            int res = puts(va_arg(ap, char*),log);
            if(res == EOF) return EOF;
            nb += res;
            ++s;
        } else if(*s == 'c') {
            //integer types smaller than int are promoted to int
            //when used in a ...
            char c = va_arg(ap, int);
            if(putc(c,log)== EOF) return EOF;
            ++nb;
            ++s;
        } else {
            //omg me dunno wat to do!!!
        }
    }
    return nb;
}

void printf(bool log, const char*format,...){
    va_list ap;
    va_start(ap, format);
    vprintf(log,format, ap);
    va_end(ap);
}

const char* txtLvls[4] = {
    "\x1b[31mError:  ",
    "\x1b[33mWarning:",
    "\x1b[34mInfo:   ",
    "\x1b[32mDebug:  ",
};

const char* txtMods[LAST_LOGMOD] = {
    "           ",
    "[Init]     ",
    "[PhyMem]   ",
    "[Paging]   ",
    "[Segment]  ",
    "[Inter]    ",
    "[Screen]   ",
    "[Kbd]      ",
    "[Mouse]    ",
    "[Syscalls] ",
    "[Graphics] ",
    "[Hdd]      ",
    "[Ext2]     ",
    "[VFS]      ",
    "[RamFS]    "
};


void vlog(LogLevel lvl, LogModule mod, const char* format, va_list ap){
    if(logLvls[mod] >= lvl){
        printf(true, "%s%s ",txtMods[mod],txtLvls[lvl]);
        vprintf(true,format,ap);
        printf(true, "\n\x1b[m");
    }
    else{
        printf(false, "%s%s ",txtMods[mod],txtLvls[lvl]);
        vprintf(false,format,ap);
        printf(false, "\n\x1b[m");
    }

}
