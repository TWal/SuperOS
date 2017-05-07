#include "log.h"

constexpr bool check(int i){
    return (serLvls[i] >= logLvls[i]) and (i == 0 ? true : check(i-1));
}
static_assert(check(LAST_LOGMOD-1),"Wrong log levels");

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
    "[RamFS]    ",
    "[CmdLine]  "
};

void vlog(LogLevel lvl, LogModule mod, const char* format, va_list ap){
    if(logLvls[mod] >= lvl){
        fprintf(stdlog, "%s%s ",txtMods[mod],txtLvls[lvl]);
        vfprintf(stdlog,format,ap);
        fprintf(stdlog, "\n\x1b[m");
    }
    else{
        fprintf(stderr, "%s%s ",txtMods[mod],txtLvls[lvl]);
        vfprintf(stderr,format,ap);
        fprintf(stderr, "\n\x1b[m");
    }

}
