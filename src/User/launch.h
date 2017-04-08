#ifndef LAUNCH_H
#define LAUNCH_H

#include"../utility.h"

struct GeneralRegisters;

extern "C" [[noreturn]] void launch(u64 rip,GeneralRegisters* regs,u64 rflags);

#endif
