#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

#include "utility.h"

struct GDTEntry{
    GDTEntry();
    u16 limitLow;
    u32 baseLow : 24;
    bool accessed : 1;
    bool RW : 1;
    bool directionDown : 1;
    bool executable : 1;
    u8 one : 1;
    u8 privilege : 2;
    bool present : 1;
    u8 limitHigh : 4;
    u8 zero : 2;
    bool is32bits : 1;
    bool is4KB : 1;
    u8 baseHigh;

    void setBase(void* base);
    void setLimit(u32 limit);
    void clear();
    void setFullDataSegment();
    void setFullExecSegment();


}__attribute__((packed));
static_assert(sizeof(GDTEntry) == 8 , "GDTEntry has wrong size");

extern GDTEntry GDT[3];


class GDTDescriptor{
public :
    GDTDescriptor();
    void init();
    void stdGDT();
    void lgdt();
    void switchSegReg();
private:
    u16 _size;
    GDTEntry* _GDT;

}__attribute__((packed));




#endif
