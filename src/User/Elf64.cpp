#include "Elf64.h"

namespace Elf64 {

Elf64::Elf64(char* data, size_t size) :
    _data(data), _size(size), _head((Header*)data) {
    assert(_head->ident[EI_MAG0] == 0x7f);
    assert(_head->ident[EI_MAG1] == 'E');
    assert(_head->ident[EI_MAG2] == 'L');
    assert(_head->ident[EI_MAG3] == 'F');
    assert(_head->phentsize == sizeof(ProgramHeader));
    assert(_head->shentsize == sizeof(SectionHeader));
    for(u16 i = 0; i < _head->shnum; ++i) {
        getSectionHeader(i).offset += (u64)data;
    }
    for(u16 i = 0; i < _head->phnum; ++i) {
        getProgramHeader(i).offset += (u64)data;
    }
}

SectionHeader& Elf64::getSectionHeader(uint i) {
    assert(i < _head->shnum);
    return *(SectionHeader*)(_data + _head->shoff + i*_head->shentsize);
}

ProgramHeader& Elf64::getProgramHeader(uint i) {
    assert(i < _head->phnum);
    return *(ProgramHeader*)(_data + _head->phoff + i*_head->phentsize);
}

Header& Elf64::head() {
    return *_head;
}

void* SectionHeader::getData() {
    return (void*)offset;
}

void* ProgramHeader::getData() {
    return (void*)offset;
}

}

