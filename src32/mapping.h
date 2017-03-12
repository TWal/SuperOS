#ifndef MAPPING_H
#define MAPPING_H

#include "../src/utility.h"

struct Mapping{
    void* phy;
    void * virt;
    u32 nbPages;
};

struct Kernel {
    void* EntryPoint;
    u32 nbMapping;
    Mapping * Mappings;
};

void setUpMapping(Mapping* map);

#endif
