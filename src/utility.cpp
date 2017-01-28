#include "utility.h"

void outb(ushort port, uchar data) {
    asm("out %0, %1" : : "r"(data), "r"(port));
}

int inb(ushort port) {
    int res;
    asm("inw %1; movl %%eax, %0" : "=r"(res) : "r"(port) : "%eax");
    return res;
}
