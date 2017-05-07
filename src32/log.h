#ifndef LOADER_LOG_H
#define LOADER_LOG_H

#include"../src/log.h"

/**
   @file This file provide loader logging system

   There are four level of logging : ...

*/

void printf(bool log, const char* format, ...);
extern char logBuffer[LOADERBUFFER * 0x1000] __attribute__((section(".data.pages")));

extern size_t posInLogBuffer;

#endif
