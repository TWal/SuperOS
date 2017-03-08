#ifndef USER_ELF64_H
#define USER_ELF64_H

#include "../utility.h"

namespace Elf64 {

class SectionHeader;
class ProgramHeader;

struct Header {
    u8 ident[16];  // ELF identification 
    u16 type;      // Object file type 
    u16 machine;   // Machine type 
    u32 version;   // Object file version 
    u64 entry;     // Entry point address 
    u64 phoff;     // Program header offset 
    u64 shoff;     // Section header offset 
    u32 flags;     // Processor-specific flags 
    u16 ehsize;    // ELF header size 
    u16 phentsize; // Size of program header entry 
    u16 phnum;     // Number of program header entries 
    u16 shentsize; // Size of section header entry 
    u16 shnum;     // Number of section header entries 
    u16 shstrndx;  // Section name string table index 
};

class Elf64 : public Header {
    public:
        Elf64(char* data, size_t size);
        SectionHeader getSectionHeader(uint i);
        ProgramHeader getProgramHeader(uint i);
    private:
        friend class SectionHeader;
        friend class ProgramHeader;
        char* _data;
        size_t _size;
};

//We shouldn't need __attribute__((packed))
static_assert(sizeof(Header) == 64);

enum EIDENT {
    EI_MAG0 = 0,       // File identification
    EI_MAG1 = 1,
    EI_MAG2 = 2,
    EI_MAG3 = 3,
    EI_CLASS = 4,      // File class
    EI_DATA = 5,       // Data encoding
    EI_VERSION = 6,    // File version
    EI_OSABI = 7,      // OS/ABI identification
    EI_ABIVERSION = 8, // ABI version
    EI_PAD = 9,        // Start of padding bytes
    EI_NIDENT = 16     // Size of e_ident[ ]
};

//EI_CLASS values
const u8 ELFCLASS32 = 1; // 32-bit objects
const u8 ELFCLASS64 = 2; // 64-bit objects

//EI_DATA values
const u8 ELFDATA2LSB = 1; // Object file data structures are little-endian
const u8 ELFDATA2MSB = 2; // Object file data structures are big-endian

//Header::type values
const u16 ET_NONE = 0;        // No file type
const u16 ET_REL = 1;         // Relocatable object file
const u16 ET_EXEC = 2;        // Executable file
const u16 ET_DYN = 3;         // Shared object file
const u16 ET_CORE = 4;        // Core file
const u16 ET_LOOS = 0xFE00;   // Environment-specific use
const u16 ET_HIOS = 0xFEFF;
const u16 ET_LOPROC = 0xFF00; // Processor-specific use
const u16 ET_HIPROC = 0xFFFF;

// =========================
// ===== SectionHeader =====
// =========================

struct SectionHeaderData {
    u32 name;      // Section name 
    u32 type;      // Section type 
    u64 flags;     // Section attributes 
    u64 addr;      // Virtual address in memory 
    u64 offset;    // Offset in file 
    u64 size;      // Size of section 
    u32 link;      // Link to other section 
    u32 info;      // Miscellaneous information 
    u64 addralign; // Address alignment boundary 
    u64 entsize;   // Size of entries, if section has table 
};

static_assert(sizeof(SectionHeaderData) == 64);

class SectionHeader : public SectionHeaderData {
    public:
        SectionHeader(Elf64* parent, uint index);
        char* getName();
        void* getData();
    private:
        Elf64* _parent;
};


//SectionHeaderData::type values
const u32 SHT_NULL = 0;            // Marks an unused section header
const u32 SHT_PROGBITS = 1;        // Contains information defined by the program
const u32 SHT_SYMTAB = 2;          // Contains a linker symbol table
const u32 SHT_STRTAB = 3;          // Contains a string table
const u32 SHT_RELA = 4;            // Contains “Rela” type relocation entries
const u32 SHT_HASH = 5;            // Contains a symbol hash table
const u32 SHT_DYNAMIC = 6;         // Contains dynamic linking tables
const u32 SHT_NOTE = 7;            // Contains note information
const u32 SHT_NOBITS = 8;          // Contains uninitialized space; does not occupy any space in the file
const u32 SHT_REL = 9;             // Contains “Rel” type relocation entries
const u32 SHT_SHLIB = 10;          // Reserved
const u32 SHT_DYNSYM = 11;         // Contains a dynamic loader symbol table
const u32 SHT_LOOS = 0x60000000;   // Environment-specific use
const u32 SHT_HIOS = 0x6FFFFFFF;
const u32 SHT_LOPROC = 0x70000000; // Processor-specific use
const u32 SHT_HIPROC = 0x7FFFFFFF;

//SectionHeaderData::flags values
const u64 SHF_WRITE = 0x1;           // Section contains writable data
const u64 SHF_ALLOC = 0x2;           // Section is allocated in memory image of program
const u64 SHF_EXECINSTR = 0x4;       // Section contains executable instructions
const u64 SHF_MASKOS = 0x0F000000;   // Environment-specific use
const u64 SHF_MASKPROC = 0xF0000000; // Processor-specific use

// ========================
// ======== Symbol ========
// ========================

struct Symbol {
    u32 name;  // Symbol name 
    int type : 4;
    int binding : 4;
    u8 other;  // Reserved 
    u16 shndx; // Section table index 
    u64 value; // Symbol value 
    u64 size;  // Size of object (e.g., common) 
} __attribute__((packed));

static_assert(sizeof(Symbol) == 24);

//Symbol::binding values
const u8 STB_LOCAL = 0;   // Not visible outside the object file 
const u8 STB_GLOBAL = 1;  // Global symbol, visible to all object files
const u8 STB_WEAK = 2;    // Global scope, but with lower precedence than global symbols
const u8 STB_LOOS = 10;   // Environment-specific use
const u8 STB_HIOS = 12;
const u8 STB_LOPROC = 13; // Processor-specific use
const u8 STB_HIPROC = 15;

//Symbol::type values
const u8 STT_NOTYPE = 0;  // No type specified (e.g., an absolute symbol)
const u8 STT_OBJECT = 1;  // Data object
const u8 STT_FUNC = 2;    // Function entry point
const u8 STT_SECTION = 3; // Symbol is associated with a section
const u8 STT_FILE = 4;    // Source file associated with the object file
const u8 STT_LOOS = 10;   // Environment-specific use
const u8 STT_HIOS = 12;
const u8 STT_LOPROC = 13; // Processor-specific use
const u8 STT_HIPROC = 15;


// =========================
// ===== ProgramHeader =====
// =========================

struct ProgramHeaderData {
    u32 type;   // Type of segment 
    u32 flags;  // Segment attributes 
    u64 offset; // Offset in file 
    u64 vaddr;  // Virtual address in memory 
    u64 paddr;  // Reserved 
    u64 filesz; // Size of segment in file 
    u64 memsz;  // Size of segment in memory 
    u64 align;  // Alignment of segment 
};

static_assert(sizeof(ProgramHeaderData) == 56);

class ProgramHeader : public ProgramHeaderData {
    public:
        ProgramHeader(Elf64* parent, uint index);
        void* getData();
    private:
        Elf64* _parent;
};


//ProgramHeaderData::type
const u32 PT_NULL = 0;            // Unused entry
const u32 PT_LOAD = 1;            // Loadable segment
const u32 PT_DYNAMIC = 2;         // Dynamic linking tables
const u32 PT_INTERP = 3;          // Program interpreter path name
const u32 PT_NOTE = 4;            // Note sections
const u32 PT_SHLIB = 5;           // Reserved
const u32 PT_PHDR = 6;            // Program header table
const u32 PT_LOOS = 0x60000000;   // Environment-specific use
const u32 PT_HIOS = 0x6FFFFFFF;
const u32 PT_LOPROC = 0x70000000; // Processor-specific use
const u32 PT_HIPROC = 0x7FFFFFFF;

//ProgramHeaderData::flags
const u32 PF_X = 0x1;               // Execute permission
const u32 PF_W = 0x2;               // Write permission
const u32 PF_R = 0x4;               // Read permission
const u32 PF_MASKOS = 0x00FF0000;   // These flag bits are reserved for environment-specific use
const u32 PF_MASKPROC = 0xFF000000; // These flag bits are reserved for processor-specific use

}

#endif

