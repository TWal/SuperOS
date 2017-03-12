#ifdef SUP_OS_KERNEL
#include "Elf64.h"
#else
#include "../src/User/Elf64.h"
#endif


namespace Elf64 {

Elf64::Elf64(char* data, size_t size) :
    Header(*(Header*)data), _data(data), _size(size) {

    assert(ident[EI_MAG0] == 0x7f);
    assert(ident[EI_MAG1] == 'E');
    assert(ident[EI_MAG2] == 'L');
    assert(ident[EI_MAG3] == 'F');
    assert(phentsize == sizeof(ProgramHeaderData));
    assert(shentsize == sizeof(SectionHeaderData));
}

SectionHeader Elf64::getSectionHeader(uint i) {
    assert(i < shnum);
    return SectionHeader(this, i);
}

ProgramHeader Elf64::getProgramHeader(uint i) {
    assert(i < phnum);
    return ProgramHeader(this, i);
}


SectionHeader::SectionHeader(Elf64* parent, uint index) :
    SectionHeaderData(*(SectionHeaderData*)(parent->_data + parent->shoff + index*parent->shentsize)),
    _parent(parent) {}

char* SectionHeader::getName() {
    return (char*)_parent->getSectionHeader(_parent->shstrndx).getData() + name;
}

void* SectionHeader::getData() {
    return _parent->_data + offset;
}


ProgramHeader::ProgramHeader(Elf64* parent, uint index) :
    ProgramHeaderData(*(ProgramHeaderData*)(parent->_data + parent->phoff + index*parent->phentsize)),
    _parent(parent) {}

void* ProgramHeader::getData() {
    return _parent->_data + offset;
}

}

