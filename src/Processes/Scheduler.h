#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../utility.h"
#include "../Interrupts/Interrupt.h"
#include "Process.h"

class Scheduler{
public :
    Scheduler();
    void init();
    void run(); // run the next program, the previous context should have been saved
};

void timerHandler(const InterruptParams&);

extern Scheduler schedul;


#endif
