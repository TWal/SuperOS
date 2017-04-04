#ifndef TASKSEGEMNT_H
#define TASKSEGEMNT_H

#include "../utility.h"

struct TSS{
    u32 zero1;
    void*RSP[3];
    u64 zero2;
    void* IST[7];
    u64 zero3;
    u16 zero4;
    u16 offset;
    u64 ones;
    TSS();
    void load();
}__attribute__((packed));
static_assert(sizeof(TSS) == 0x70,"Wrong size for TSS");

struct TSSDescriptor{
    u16 limitLow;
    u32 baseLow : 24;
    u8 type : 4; // 0x9
    u8 zero1 : 1;
    u8 privilege : 2;
    bool present : 1;
    u8 limitHigh : 4;
    u8 zero2 : 3;
    bool is4KB : 1;
    u64 baseHigh : 40;
    u32 zero3;
    TSSDescriptor();
    void setBase(void* addr);
    void setLimit(u32 limit);
}__attribute__((packed));
static_assert(sizeof(TSSDescriptor) == 16,"Wrong size for TSSDescriptor");


extern TSS tss;





#endif
