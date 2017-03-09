#include "Segmentation.h"



GDTEntry GDT[3];


GDTEntry::GDTEntry(): limitLow(0),baseLow(0),accessed(0),RW(true),
                      directionDown(0),executable(false),one(1),
                      privilege(0),present(false),limitHigh(0),
                      zero(0),is32bits(true),is4KB(true),baseHigh(0)
{

}



void GDTEntry::setBase(void* base){
    baseLow = reinterpret_cast<uint>(base) & 0x00FFFFFF;
    baseHigh = static_cast<uchar>(reinterpret_cast<uint>(base)>>24);

}

void GDTEntry::setLimit(uint limit){
    limitLow = limit & 0x0000FFFF;
    limitHigh = static_cast<uchar>(reinterpret_cast<uint>(limit)>>16);
}

void GDTEntry::clear(){
    GDTEntry();
    RW = false;
    one = 0;
    is32bits = false;
    is4KB = false;
}

void GDTEntry::setFullDataSegment(){
    setBase(nullptr);
    setLimit(0xFFFFF);
    present = true;
}
void GDTEntry::setFullExecSegment(){
    setBase(nullptr);
    setLimit(0xFFFFF);
    present = true;
    executable =true;
}

GDTDescriptor::GDTDescriptor(){
}

void GDTDescriptor::init(){
    stdGDT();
    lgdt();
    switchSegReg();
}


void GDTDescriptor::stdGDT(){
    GDT[0].clear();
    GDT[1].setFullExecSegment();
    GDT[2].setFullDataSegment();
}

void GDTDescriptor::lgdt(){
    _size = 3*8;
    _GDT = GDT;
    asm volatile (
        "lgdt (%0)" :
        : "r"(this):
        );
}
void __attribute__((optimize("O0"))) GDTDescriptor::switchSegReg(){
    asm volatile (
        "ljmp $0x08 , $switchSegRes_rec;"
        "switchSegRes_rec : \n"
        "mov $0x10,%%eax;"
        "mov %%eax,%%ds;"
        "mov %%eax,%%ss;"
        "mov %%eax,%%es;":
        : : "%eax"
        );
}

GDTDescriptor gdt;