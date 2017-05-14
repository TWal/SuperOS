#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "../utility.h"

/**
   @brief This class represent an entry in the @ref IDT.

   @sa @ref inter, InterruptTable
 */
struct InterruptEntry {
    u16 offLow; ///< The 16 lower bits of the address
    u16 segSelector; ///< The segment in which the handler must be executed
    char ist : 3; ///< The ist pointer (see @ref inter)
    char zero :5;
    bool isTrap : 1; /// If the entry is a trap
    char ones :3;
    char zero2 : 1;
    char privilegeLvl :2; ///< The minimum privilege required to activate the interrupt.
    bool present : 1; ///< If this entry is active (doublefault is not)
    u64 offHigh : 48; ///< the 48 higher bits of the address.
    u32 zero3;

    /**
       @biref Initialize to default value :
           - Segment : 0x08 (@ref CODE_SEGEMENT)
           - Ist : 0 i.e default handling
           - Trap : interrupt gate by default
           - Privilege level : ring 3.
           - Present :  false.
     */
    InterruptEntry();
    void setAddr(void* addr); ///< Set the handler address
    void* getAddr(); ///< get the handler address

}__attribute__((packed));

static_assert(sizeof(InterruptEntry) == 16 , "InterruptEntry has wrong size");

/// The Interrupt Descriptor Table, see @ref inter
extern InterruptEntry IDT[256];

/// Loads the @ref IDT in the itdr register.
void lidt();

/**
   @brief The input parameter of an non-error interrupt handler.

   This value contains the value of all the register and should not be modified
   (const_cast) if you don't know exactly what you are doing

   @sa @ref inter, InterruptTable
 */
struct InterruptParams{
    u64 zero;
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rsp;
    u64 rdx;
    u64 rax;
    u64 rcx;
    u64 rflags;
    u64 rbx;
    void*  rip;
    u64 cs;
    u64 flags;
    u64 oldrsp;
    u64 oldss;
}__attribute__((packed));


/**
   @brief The input parameter of an error interrupt handler.

   This value contains the value of all the register and should not be modified
   (const_cast) if you don't know exactly what you are doing

   @sa @ref inter, InterruptTable
*/
struct InterruptParamsErr{
    u64 zero;
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rsp;
    u64 rdx;
    u64 rax;
    u64 rcx;
    u64 rflags;
    u64 rbx;
    u64 errorCode;
    void*  rip;
    u64 cs;
    u64 flags;
    u64 oldrsp;
    u64 oldss;
}__attribute__((packed));

/**
   @brief This struct describe specific characteristic of an interrupt handler
   that are not related to the processor

   They are contained in @ref params.
 */
struct IntParams{
    bool isAssembly : 1;
    bool doReturn : 1;
    bool hasErrorCode : 1;
    IntParams(): isAssembly(false),doReturn(false),hasErrorCode(false){};
}__attribute__((packed));

static_assert(sizeof(IntParams) == 1, "IntParams is too big");

/// Type of a handler that take a non-error code interruption and return something
using interFuncR = u64(*)(const InterruptParams&);
/// Type of a handler that take a non-error code interruption and do not return
using interFunc = void(*)(const InterruptParams&);
/// Type of a handler that take a error code interruption and return something
using interFuncER = u64(*)(const InterruptParamsErr&);
/// Type of a handler that take a error code interruption and do not return
using interFuncE = void(*)(const InterruptParamsErr&);

extern "C" interFunc intIDT[256];
extern "C" void initIntIDT ();
extern "C" IntParams params [256];

/**
   @brief This class provides a abstraction to manipulate CPU interruption handler.
 */
class InterruptTable {
public:
    /// Setup interruption table, IF must be false before calling this method.
    void init();
    void addInt(u8 i,interFunc f);
    void addInt(u8 i,interFuncR f);
    void addInt(u8 i,interFuncE f);
    void addInt(u8 i,interFuncER f);
    /// Add an assembly handler, do not furnish C/C++ abstraction.
    void addIntAsm(u8 i, void* f);
    /// Set interrupt i in trap gate mode
    void setTrapGate(u8 i);
    /// Set interrupt i in interrupt gate mode (This is the default mode)
    void setInterruptGate(u8 i);
    InterruptTable();
    void allPresent();// only for debugging purpose
};



template<u8 num>
void interrupt(){
    asm volatile(
        "int %0;" :
        : "i"(num) :
        );
}

template<u8 num>
void interrupt(u64 rax){
    asm volatile(
        "mov %1,%%rax;"
        "int %0;" :
        : "i"(num) , "r"(rax): "%rax"
        );
}

template<u8 num>
void interrupt(u64 rax,u64 rbx){
    asm volatile(
        "mov %1,%%rax;"
        "mov %2,%%rbx;"
        "int %0;" :
        : "i"(num) , "r"(rax),"r"(rbx)
        : "%rax", "%rbx"
        );
}

template<u8 num>
u64 interruptr(){
    u64 res;
    asm volatile(
        "int %1;"
        "mov %%rax,%0;"
        :"=r"(res)
        :"i"(num)
        :"%rax"
        );
    return res;
}

template<u8 num>
u64 interruptr(u64 rax){
    u64 res;
    asm volatile(
        "mov %2,%%rax;"
        "int %1;"
        "mov %%rax,%0;"
        :"=r"(res)
        :"i"(num), "r"(rax)
        :"%rax"
        );
    return res;
}

template<u8 num>
u64 interruptr(u64 rax,u64 rbx){
    u64 res;
    asm volatile(
        "mov %2,%%rax;"
        "mov %3,%%rbx;"
        "int %1;"
        "mov %%rax,%0"
        :"=r"(res)
        : "i"(num) , "r"(rax),"r"(rbx)
        : "%rax", "%rbx"
        );
    return res;
}

extern InterruptTable idt;


/**
   @page inter Interrupt System

   @biref In x86_64 architecture an interrupt is a event this needs immediate handling.

   @section inter_proc Processor interface

   In protected mode and in long mode, there is an Interrupt Descriptor table
   (@ref IDT) of size 256. Thus there are 256 possible interrupts and when the
   interrupt number i happen, the handler IDT[i] is called.

   The are two type of interrupts : Trap gates and Interrupt gate. When you are
   in a Interrupt gate, no other interrupts are allowed, when you are in a Trap
   gate, other interrupt cans happen (see @ref InterruptEntry::isTrap).

   @section inter_supos Interrupts in Super OS

   Normally, an interrupts handler must preserve all registers (excepted possibly rax
   for return value), The it cannot be directly a C function. Thus in Interrupt.s
   there is a generic handler that save register and call a C -function, the one in
   @ref intIDT.

   The values in @ref params determine if their is a return value or if rax must
   be restored (@ref IntParams::doReturn) and if there is an error code
   (@ref IntParams::hasErrorCode).

 */



#endif
