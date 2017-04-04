#include "Segmentation.h"



GDTEntry GDT[gdtsize];


GDTEntry::GDTEntry(): limitLow(0),baseLow(0),accessed(0),RW(true),
                      directionDown(0),executable(false),one(1),
                      privilege(0),present(false),limitHigh(0),
                      zero(0),is64bits(true),is32bits(false),is4KB(true),baseHigh(0)
{

}

void GDTEntry::clear(){
    *reinterpret_cast<u64*>(this) = 0;
}

void GDTEntry::setFullDataSegment(){
    present = true;
}
void GDTEntry::setFullExecSegment(){
    present = true;
    executable =true;
}

GDTDescriptor::GDTDescriptor(){
}

extern "C" void switchSegReg();

void GDTDescriptor::init(){
    stdGDT();
    lgdt();
    switchSegReg();
}


void GDTDescriptor::stdGDT(){
    GDT[0].clear();
    GDT[1].setFullExecSegment();
    GDT[2].setFullDataSegment();
    GDT[3].setFullExecSegment();
    GDT[3].privilege = 3;
    GDT[3].is64bits = false;
    GDT[3].is32bits = true;
    GDT[4].setFullDataSegment();
    GDT[4].privilege = 3;
    GDT[5].setFullExecSegment();
    GDT[5].privilege = 3;
}

void GDTDescriptor::lgdt(){
    _size = gdtsize*8;
    _GDT = GDT;
    asm volatile (
        "lgdt (%0)" :
        : "r"(this):
        );
}

GDTDescriptor gdt;
