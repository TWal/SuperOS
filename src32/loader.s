    .global loader
    .global enableLM
    .global startKernel

    .equ MAGIC_NUMBER, 0x1BADB002
    .equ FLAGS, 0x3 #information about memory + aligned modules
    .equ CHECKSUM, -MAGIC_NUMBER - FLAGS
    ## MAGICNUMBER + FLAGS + CHECKSUM = 0

    .equ STACKSIZE, 1024 * 16   # stack + heap


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
    ## checking long mode
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
    push %eax
    push %ebx
    push %ecx
    push %edx
    mov 20(%esp),%eax           # Argument is PML4 physical address
    mov %eax, %cr3

    ## set the PAE bit
    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    ## switch to long mode
    mov $0xC0000080,%ecx
    rdmsr
    or $(1 << 8)|1,%eax
    wrmsr

    ## set PG
    mov %cr0, %ebx
    or $0x80000000, %ebx
    mov %ebx, %cr0

    pop %edx
    pop %ecx
    pop %ebx
    pop %eax

    ret

invProc:                        #TODO real error handling (bsod).
	    cli
	    hlt


startKernel:
    ljmp $0x18, $startKernel64  # jumping to 64-bit mode

.code64
startKernel64:
    movq 4(%rsp),%rax
    movl 12(%rsp),%edi
    movl 12(%rsp),%esp          # TODO This is stupid.
    jmp *%rax # launching kernel



