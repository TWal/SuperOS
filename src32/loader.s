    .global loader
    .global enableLM
    .global startKernel

    .equ MAGIC_NUMBER, 0x1BADB002
    .equ FLAGS, 0x2 #information about memory
    .equ CHECKSUM, -MAGIC_NUMBER - FLAGS
    ## MAGICNUMBER + FLAGS + CHECKSUM = 0

    .equ STACKSIZE, 4096


.bss
loader_stack:
    .global loader_stack
    .space STACKSIZE


.section .text
    .int MAGIC_NUMBER, FLAGS, CHECKSUM

loader:
    cli
    mov $(STACKSIZE + loader_stack),%esp
    push %ebx                 # multiboot info pointer is in %ebx
    ## setting up long mode
    mov $0x80000000,%eax      # is there high code request ?
    cpuid                     # CPU identification.
    cmp $0x80000001, %eax     # Compare the A-register with 0x80000001.
    jb invProc
    mov $0x80000001,%eax
    cpuid
    test $(1<<29),%edx
    jz invProc

    call load

enableLM:
    ## give the address of PDE
                                #mov $PML4lower, %eax
    mov 4(%esp),%eax
    mov %eax, %cr3

    ## set the PAE bit
    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    ## switch to long mode
    mov $0xC0000080,%ecx
    rdmsr
    or $(1 << 8),%eax
    wrmsr

    ## set PG
    mov %cr0, %ebx
    or $0x80000000, %ebx
    mov %ebx, %cr0

    ret

startKernel:
    xchg %bx,%bx
    ljmp $0x18, $startKernel64

.code64
startKernel64:
    mov $4,%rax
    shl $40,%rax
    jmp invProc



invProc:
    cli
    hlt
