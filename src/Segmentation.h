#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

#include "utility.h"

struct GDTEntry{
    GDTEntry();
    ushort limitLow;
    uint baseLow : 24;
    bool accessed : 1;
    bool RW : 1;
    bool directionDown : 1;
    bool executable : 1;
    uchar one : 1;
    uchar privilege : 2;
    bool present : 1;
    uchar limitHigh : 4;
    uchar zero : 2;
    bool is32bits : 1;
    bool is4KB : 1;
    uchar baseHigh;

    void setBase(void* base);
    void setLimit(uint limit);
    void clear();
    void setFullDataSegment();
    void setFullExecSegment();


}__attribute__((packed));
static_assert(sizeof(GDTEntry) == 8 , "GDTEntry has wrong size");

extern GDTEntry GDT[3];


struct GDTDescriptor{
    ushort size;
    GDTEntry* GDT;

}__attribute__((packed));

void stdGDT();

void lgdt();
void switchSegReg();



#endif
