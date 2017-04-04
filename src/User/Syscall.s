    .global syscall
syscall:
    mov %rsp, 0xFFFFFFFFFFFFFFF8
    movabs $0xFFFFFFFFFFFFFFF8,%rsp
    ## Stack is operational
    push %rcx
    mov %r10,%rcx
    ## Calling convention is now respected
    push %rbx
    push %rdx
    push %rdi
    push %rsi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11                   # Contains RFLAGS
    push %r12
    push %r13
    push %r14
    push %r15
    mov handlers,%rbx
    mov $-1,%rcx
    testq $-1,(%rbx,%rax,8)     # test if the pointer is activated (non zero)
    cmovzq %rcx,%rax              # if not call handler -1
    call *(%rbx,%rax,8)
    xchg %bx,%bx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rsi
    pop %rdi
    pop %rdx
    pop %rbx
    pop %rcx                    # Return RIP
    pop %rsp
    sysretq

