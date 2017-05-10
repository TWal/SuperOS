#include "Context.h"
#include "../Interrupts/Interrupt.h"
#include "../log.h"

Context::Context(const InterruptParams& params){
    assert((i64)params.rip > 0 && params.cs == 0x2b);  // we are in user mode
    *this = *reinterpret_cast<const Context*>(&params);
}
Context& Context::operator= (const InterruptParams& params){
    assert((i64)params.rip > 0 && params.cs == 0x2b);  // we are in user mode
    *this = *reinterpret_cast<const Context*>(&params);
    return *this;
}
extern "C"  void launcht(Context* context)__attribute__((noreturn));

void Context::launch(){
    launcht(this);
}

void Context::save(const InterruptParams& params){
    assert((i64)params.rip > 0 && params.cs == 0x2b);  // we are in user mode
    lastContext = reinterpret_cast<Context*>(const_cast<InterruptParams*>(&params));
}

Context* Context::lastContext;
