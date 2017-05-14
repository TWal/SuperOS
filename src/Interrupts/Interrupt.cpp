#include "Interrupt.h"

InterruptEntry IDT[256];

InterruptEntry::InterruptEntry() : offLow(0),segSelector(0x08),ist(0), zero(0),
                                   isTrap(false),ones(-1),zero2(0),privilegeLvl(3),
                                   present(false),offHigh(0),zero3(0)
{
}

void InterruptEntry::setAddr(void* addr){
    offLow = static_cast<ushort> (reinterpret_cast<uptr>(addr));
    offHigh = static_cast<u64>(reinterpret_cast<uptr>(addr)>>16);
}

void* InterruptEntry::getAddr(){
    return reinterpret_cast<void*>
        ((offHigh <<16) + static_cast<uint>(offLow));
}

struct IDTLoader{
    u16 size;
    InterruptEntry * IDT;
}__attribute__((packed));

void __attribute__((optimize("O0"))) lidt (){
    IDTLoader lo = {256*16,IDT};
    asm volatile (
        "lidt %0" :
        : "m"(lo)
        );
}



InterruptTable::InterruptTable(){
    initIntIDT(); // load the _inter_... in intIDT[...]
}

void InterruptTable::init(){
    assert(this == &idt);
    for(u16 i = 0 ; i < 256 ; ++i){
        IDT[i].setAddr(reinterpret_cast<void*>(intIDT[i]));
        intIDT[i] = nullptr;
    }
    lidt();
}

void InterruptTable::addInt(u8 i,interFunc f){
    intIDT[i] = f;
    IDT[i].present =true;
}
void InterruptTable::addInt(u8 i,interFuncR f){
    intIDT[i] = reinterpret_cast<interFunc>(f);
    params[i].doReturn = true;
    IDT[i].present = true;
}
void InterruptTable::addInt(u8 i,interFuncE f){
    intIDT[i] = reinterpret_cast<interFunc>(f);
    params[i].hasErrorCode = true;
    IDT[i].present = true;
}
void InterruptTable::addInt(u8 i,interFuncER f){
    intIDT[i] = reinterpret_cast<interFunc>(f);
    params[i].doReturn = true;
    params[i].hasErrorCode = true;
    IDT[i].present = true;
}

void InterruptTable::addIntAsm(u8 i,void* f){
    params[i].isAssembly = true;
    IDT[i].present = true;
    IDT[i].setAddr(f);
}

void InterruptTable::setTrapGate(u8 i){
    IDT[i].isTrap = true;
}
void InterruptTable::setInterruptGate(u8 i){
    IDT[i].isTrap = false;
}

void InterruptTable::allPresent(){
    for(u16 i = 0 ; i < 256 ; ++i){
        IDT[i].present = true;
    }
}

InterruptTable idt;
