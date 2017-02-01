#ifndef GLOBALS_H
#define GLOBALS_H

#include "FrameBuffer.h"
#include "Pic.h"
#include "Segmentation.h"
#include "Interrupt.h"
#include "Keyboard.h"

extern FrameBuffer fb;
extern Pic pic;
extern GDTDescriptor gdt;
extern InterruptTable idt;
extern Keyboard kbd;

#endif

