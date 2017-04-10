#include "Context.h"
#include "../Interrupts/Interrupt.h"

Context::Context(InterruptParams* params){
    assert((i64)params->rip > 0 && params->cs == 0x2b);  // we are in user mode
    *this = *reinterpret_cast<Context*>(params);
}
extern "C"  void launcht(Context* context)__attribute__((noreturn));

void Context::launch(){
    launcht(this);
}

void Context::save(InterruptParams* params){
    assert((i64)params->rip > 0 && params->cs == 0x2b);  // we are in user mode
    lastContext = reinterpret_cast<Context*>(params);
}

Context* Context::lastContext;
