#include "Interrupt.h"

InterruptEntry IDT[256];

InterruptEntry::InterruptEntry() : offLow(0),segSelector(0x08),reserved0(0),
                                 reserved1(6),is32bits(true),reserved2(0),
                                 privilegeLvl(0),present(false),offHigh(0)
{
}

void InterruptEntry::setAddr(void* addr){
    offLow = static_cast<ushort> (reinterpret_cast<uint>(addr));
    offHigh = static_cast<ushort>(reinterpret_cast<uint>(addr)>>16);
}

void* InterruptEntry::getAddr(){
    return reinterpret_cast<void*>(
        (static_cast<uint>(offHigh) <<16) + static_cast<uint>(offLow));
}

struct IDTLoader{
    ushort size;
    InterruptEntry * IDT;
}__attribute__((packed));

void lidt (){
    IDTLoader lo = {256*8,IDT};
    asm volatile (
        "lidt (%0)" :
        : "r"(&lo):
        );
}

