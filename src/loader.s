.global loader
.global loaderbefore

.equ MAGIC_NUMBER, 0x1BADB002
.equ FLAGS, 0x2 #information about memory
.equ CHECKSUM, -MAGIC_NUMBER - FLAGS
# MAGICNUMBER + FLAGS + CHECKSUM = 0

.equ HEAPSTACKSIZE, 2048576
.equ STACKSIZE, 4096

.bss
kernel_heapstack:
  .global kernel_heapstack
    .space HEAPSTACKSIZE

.section .bss.lower
kernel_stack_lower:
    .space STACKSIZE

.section .text.lower
.int MAGIC_NUMBER, FLAGS, CHECKSUM

loaderbefore:
    mov $(STACKSIZE + kernel_stack_lower),%esp
    push %ebx # multiboot info pointer is in %ebx

    #setting up long mode
    mov $0x80000000,%eax      # is there high code request ?
    cpuid                     # CPU identification.
    cmp 0x80000001, %eax      # Compare the A-register with 0x80000001.
    jb invProc

    mov 0x80000001,%eax
    cpuid
    test $(1<<29),%edx
    jz invProc

    call setupBasicPaging
    pop %ecx #now it is in %ecx
    #give the address of PDE
    mov $PDElower, %eax
    mov %eax, %cr3

    #set the PSE bit
    mov %cr4, %ebx
    or $0x10, %ebx
    mov %ebx, %cr4

    #set PG
    mov %cr0, %ebx
    or $0x80000000, %ebx
    mov %ebx, %cr0
    lea loader, %ebx
    jmp *%ebx

invProc:
    cli
    hlt

.text
loader:
    mov $(HEAPSTACKSIZE + kernel_heapstack),%esp
    add $0xC0000000, %ecx
    push %ecx
    call kmain
    pop %ecx
    xchg %bx,%bx

loop:
    jmp loop
