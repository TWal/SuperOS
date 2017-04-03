    .global switchSegReg
switchSegReg:
    xor %rax,%rax
    pushq $0x10
	  push %rsp
    add $8,(%rsp)
	  pushfq
    pushq $0x8
	  movabs $switchSegReg_rec,%rax
	  push %rax
	  iretq
switchSegReg_rec :
	  mov $0x10,%eax
	  mov %eax,%ds
	  mov %eax,%ss
	  mov %eax,%es
    ret
