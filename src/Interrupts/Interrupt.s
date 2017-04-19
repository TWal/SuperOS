.bss
    .global intIDT
intIDT:
    .space 8*256

    .global params
params:
    .space 256



.text
    .global geneInt
geneInt: # by default interruption on user stack.
    ## %rbx contains the number of the interruption, all the others are not changed.
    push 0x18(%rsp)             #re push flags for compatibility with context structure
    push %rcx
    push %rax
    push %rdx
    push 0x40(%rsp)             #push oldrsp
    push %rbp
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    push $0                     #for mmx saving if needed
    mov %rsp,%rdi
    movabs $intIDT, %rbp
    call *(%rbp,%rbx,8)         #%rbx is callee-saved
    add $8,%rsp
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
    pop %rcx
    testb $2,params(,%rbx)
    cmovz %rcx,%rax
    pop %rcx
    add $8,%rsp
    testb $4,params(,%rbx)
    jnz errEnd
    pop %rbx
    iretq

errEnd:
    pop %rbx
    add $8,%rsp
    iretq
