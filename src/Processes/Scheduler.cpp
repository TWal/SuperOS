#include "Scheduler.h"
#include "../Interrupts/Pic.h"
#include "../Interrupts/Pit.h"
#include <stdio.h>

Scheduler::Scheduler(){
        Pit::set(0,1000,Pit::SQUAREWAVE);

}

void Scheduler::init(){
    idt.addInt(0x20,timerHandler);
    pic.activate(Pic::TIMER);
}
void timerHandler(const InterruptParams&){
    // do smart stuff
    static int i = 0;
    if(i % 1000 == 0) printf("Timer Interrupt");
    ++i;
    pic.endOfInterrupt(0);
}

Scheduler schedul;
