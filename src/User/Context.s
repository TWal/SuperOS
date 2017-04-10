    .global launcht
launcht:
    xchg %bx,%bx
    pushq $0x23
	  push 0x60(%rdi)           #this is target rsp
	  push 0x80(%rdi)           # push flags
	  pushq $0x2b
	  push 0x90(%rdi)           #this is target rip
    mov 0x8(%rdi),%r15
    mov 0x10(%rdi),%r14
    mov 0x18(%rdi),%r13
    mov 0x20(%rdi),%r12
    mov 0x28(%rdi),%r11
    mov 0x30(%rdi),%r10
    mov 0x38(%rdi),%r9
    mov 0x40(%rdi),%r8
    #mov 0x48(%rdi),%rdi
    mov 0x50(%rdi),%rsi
    mov 0x58(%rdi),%rbp
    ## %rsp
    mov 0x68(%rdi),%rdx
    mov 0x70(%rdi),%rax
    mov 0x78(%rdi),%rcx
    mov 0x88(%rdi),%rbx
    mov 0x48(%rdi),%rdi
    iretq
