#include"../src/Interrupts/Interrupt.h"

u64 add(const InterruptParams& p){
    return p.rax + p .rbx;
}

void print(const InterruptParams&p){
    printf("rax : %llu",p.rax);
}

void unittest(){
    idt.addInt(142,print);
    idt.addInt(217,add);
    interrupt<142>(interruptr<217>(40,2));
}

