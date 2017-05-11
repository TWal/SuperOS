#ifndef LOG_H
#define LOG_H

#include "utility.h"

/**
   @file This file provide kernel logging system

   There are four level of logging : ...

 */


/// Size of buffer for log before boot.
#define LOADERBUFFER 2
#define KERNELBUFFER 3


enum LogLevel{Error, Warning, Info, Debug};

enum LogModule{Generic, Init, PhyMem, Pagingl, Segment, Inter, Screenl, Kbd, Mousel,
               Syscalls, Schedul, Proc, Graphics, Hdd, Ext2, VFS, RamFS, CmdLine,
               LAST_LOGMOD};

constexpr LogLevel serLvls[LAST_LOGMOD] =
{
    Debug, // Generic
    Debug, // Init
    Debug, // PhyMem
    Info, // Pagingl
    Debug, // Segment
    Debug, // Inter
    Debug, // Screenl
    Info,// Kbd
    Debug, // Mousel
    Debug, // Syscalls
    Info, // Schedul
    Info, // Proc
    Debug, // Graphics
    Debug, // Hdd
    Debug, // Ext2
    Debug, // VFS
    Debug, // RamFS
    Debug, // CmdLine
};

constexpr LogLevel logLvls[LAST_LOGMOD] =
{
    Debug, // Generic
    Info,  // Init
    Info, // PhyMem
    Info, // Pagingl
    Debug, // Segment
    Debug, // Inter
    Debug, // Screenl
    Info, // Kbd
    Debug, // Mousel
    Debug, // Syscalls
    Warning, // Schedul
    Info, // Proc
    Debug, // Graphics
    Debug, // Hdd
    Debug, // Ext2
    Debug, // VFS
    Debug, // RamFS
    Debug, // CmdLine
};

void vlog(LogLevel lvl, LogModule mod, const char* format, va_list ap);

inline void log(LogLevel lvl, LogModule mod, const char* format, ...){
    if(serLvls[mod] >= lvl){
        va_list ap;
        va_start(ap, format);
        vlog(lvl,mod,format,ap);
        va_end(ap);
    }
}

inline void error(const char* format, ...){
    va_list ap;
    va_start(ap, format);
    vlog(Error,Generic,format,ap);
    va_end(ap);
}
inline void warning( const char* format, ...){
    if(serLvls[Generic] >= Warning){
        va_list ap;
        va_start(ap, format);
        vlog(Warning,Generic,format,ap);
        va_end(ap);
    }
}
inline void info(const char* format, ...){
    if(serLvls[Generic] >= Info){
        va_list ap;
        va_start(ap, format);
        vlog(Info,Generic,format,ap);
        va_end(ap);
    }
}
inline void debug(const char* format, ...){
    if(serLvls[Generic] >= Debug){
        va_list ap;
        va_start(ap, format);
        vlog(Debug,Generic,format,ap);
        va_end(ap);
    }
}

inline void error(LogModule mod, const char* format, ...){
    va_list ap;
    va_start(ap, format);
    vlog(Error,mod,format,ap);
    va_end(ap);
}
inline void warning(LogModule mod, const char* format, ...){
    if(serLvls[mod] >= Warning){
        va_list ap;
        va_start(ap, format);
        vlog(Warning,mod,format,ap);
        va_end(ap);
    }
}
inline void info(LogModule mod, const char* format, ...){
    if(serLvls[mod] >= Info){
        va_list ap;
        va_start(ap, format);
        vlog(Info,mod,format,ap);
        va_end(ap);
    }
}
inline void debug(LogModule mod, const char* format, ...){
    if(serLvls[mod] >= Debug){
        va_list ap;
        va_start(ap, format);
        vlog(Debug,mod,format,ap);
        va_end(ap);
    }
}








#endif
