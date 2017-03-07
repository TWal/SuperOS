#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

#include "../src/utility.h"

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
    u8 zero : 1;
    bool is64bits : 1;
    bool is32bits : 1;
    bool is4KB : 1;
    u8 baseHigh;

    void setBase(void* base);
    void setLimit(u32 limit);
    void clear();
    void setFullDataSegment();
    void setFullExecSegment();
    void set64bits();


}__attribute__((packed));
static_assert(sizeof(GDTEntry) == 8 , "GDTEntry has wrong size");

extern GDTEntry GDT[5];

#define EXEC32BITS 0x08
#define DATA32BITS 0x10
#define EXEC64BITS 0x18
#define DATA64BITS 0x20


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

extern GDTDescriptor gdt;


#endif
