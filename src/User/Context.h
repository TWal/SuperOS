#ifndef CONTEXT_H
#define CONTEXT_H

class Context{
    // lots of things.
    Context();
    static Context getCurrentContext();
};

extern "C" void testLaunchCPL3();

#endif
