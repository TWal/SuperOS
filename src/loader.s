.global loader
.global loaderbefore

.equ MAGIC_NUMBER, 0x1BADB002
.equ FLAGS, 0x0
.equ CHECKSUM, -MAGIC_NUMBER - FLAGS
# MAGICNUMBER + FLAGS + CHECKSUM = 0

.equ STACKSIZE, 4096

.bss
kernel_stack:
    .space STACKSIZE

.section .bss.lower
kernel_stack_lower:
    .space STACKSIZE

.section .text.lower
.int MAGIC_NUMBER, FLAGS, CHECKSUM

loaderbefore:
    mov $(STACKSIZE + kernel_stack_lower),%esp
    call setupPaging
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

.text
loader:
    mov $(STACKSIZE + kernel_stack),%esp
    call kmain
    xchg %bx,%bx
loop:
    jmp loop
