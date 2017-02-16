#include "globals.h"

GDTDescriptor gdt;
InterruptTable idt;
FrameBuffer fb;
Pic pic;
Keyboard kbd;
multibootInfo multiboot;
PhysicalMemoryAllocator physmemalloc;
Paging paging;
