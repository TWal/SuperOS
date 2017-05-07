    .global loader
    .global LMcheck
    .global enableLM
    .global startKernel

    .equ MAGIC_NUMBER, 0x1BADB002
    .equ FLAGS, 0x7 #information about memory + aligned modules + graph
    .equ CHECKSUM, -MAGIC_NUMBER - FLAGS
    ## MAGICNUMBER + FLAGS + CHECKSUM = 0

    .equ STACKSIZE, 1024 * 16   # stack + heap


.bss
loader_stack:
    .global loader_stack
    .space STACKSIZE


.section .text
    .int MAGIC_NUMBER, FLAGS, CHECKSUM
    .int 0,0,0,0,0
    .int 0,1024,768,32

loader:
    cli
    mov $(STACKSIZE + loader_stack),%esp
    push %ebx                 # multiboot info pointer is in %ebx
    call load


LMcheck:
    push %eax
    push %edx
    ## checking long mode
	mov $0x80000000,%eax      # is there high code request ?
	cpuid                     # CPU identification.
	cmp $0x80000001, %eax     # Compare the A-register with 0x80000001.
	jb LMunav
	mov $0x80000001,%eax
	cpuid
	test $(1<<29),%edx
    jz LMunav
    pop %edx
    pop %eax
    ret


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


startKernel:
    ljmp $0x18, $startKernel64  # jumping to 64-bit mode

.code64
startKernel64:
    movq 4(%rsp),%rax
    movl 12(%rsp),%edi
    movl 12(%rsp),%esp          # TODO This is stupid.
    jmp *%rax # launching kernel




