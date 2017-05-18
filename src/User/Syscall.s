    .global syscall
syscall:
    #xchg %bx,%bx
    mov %rsp, -0x38
    xor %rsp,%rsp               # zero %rsp
    ## Stack is operational
    push %rcx                   #push caller %rip
    mov %r10,%rcx
    ## Calling convention is now respected
    push %rbx
    push %r11                   # push RFLAGS
    pushq $0                    # %rcx
    push %rax
    push %rdx
    sub $8,%rsp                 #pas over previously saved %rsp
    push %rbp
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push $0                     # %r11
    push %r12
    push %r13
    push %r14
    push %r15
    push $0                     # pointer for saving x87/SSE/MMX
    push $0                     # 16-bytes aligned stack for x64 ABI
    call syssave
    mov handlers,%rbx
    mov $-1,%rcx
    testq $-1,(%rbx,%rax,8)     # test if the pointer is activated (non zero)
    cmovzq %rcx,%rax              # if not call handler -1
    call *(%rbx,%rax,8)
    add $0x8,%rsp
    mov %rax,%rdi
    push %rax
    call sysleave
    pop %rax
    add $0x8,%rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rbp
    add $8,%rsp
    pop %rdx
    add $8,%rsp
    pop %rcx
    pop %r11                    # get back RFLAGS
    pop %rbx
    pop %rcx                    # get back %rip
    mov -0x38,%rsp              # user stack is in place
    sysretq

