#include"TaskSegment.h"
#include"../Memory/Segmentation.h"

TSS::TSS():zero1(0),RSP{},zero2(0),IST{},
           zero3(0),zero4(0),offset(0x68),ones(-1){}

TSS tss;

void TSS::load(){
    TSSDescriptor* desc = reinterpret_cast<TSSDescriptor*>(&GDT[TSS_SEGMENT/8]);
    new(desc)TSSDescriptor();
    desc->setBase(&tss);
    desc->setLimit(sizeof(tss));
    asm volatile("ltr %0" : : "r"((ushort)TSS_SEGMENT) :);
}

TSSDescriptor::TSSDescriptor() : limitLow(0),baseLow(0),type(0x9),zero1(0),
                                 privilege(3),present(true),limitHigh(0),zero2(0),
                                 is4KB(false),baseHigh(0),zero3(0){}

void TSSDescriptor::setBase(void* addr){
    baseLow = (uptr)addr & 0xFFFFFF;
    baseHigh = (uptr)addr >> 24;
}
void TSSDescriptor::setLimit(u32 limit){
    limit &= 0xFFFFFF;
    limitLow = limit & 0xFFFF;
    limitHigh = limit >>16;
}
