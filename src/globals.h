#ifndef GLOBALS_H
#define GLOBALS_H

#include "FrameBuffer.h"
#include "Pic.h"
#include "Segmentation.h"
#include "Interrupt.h"

extern FrameBuffer fb;
extern Pic pic;
extern GDTDescriptor gdt;
extern InterruptTable idt;

#endif

