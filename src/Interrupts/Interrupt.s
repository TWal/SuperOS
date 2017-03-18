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
    push %rbp
    push %rsi
    push %rdi
    push %rdx
    push %rcx
    push %rax
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
  	mov %rsp,%rdi
    movabs $intIDT, %rbp
    call *(%rbp,%rbx,8)         #%rbx is callee-saved
 	  pop %r15
	  pop %r14
	  pop %r13
	  pop %r12
	  pop %r11
	  pop %r10
	  pop %r9
	  pop %r8
    pop %rcx
    movabs $params, %rbp
    testb $2,(%rbp,%rbx,8)
    cmovz %rcx,%rax
    pop %rcx
    pop %rdx
    pop %rdi
	  pop %rsi
	  pop %rbp
    testb $4,(%rbp,%rbx,8)
	  jnz errEnd
    pop %rbx
    iretq

errEnd:
	  pop %rbx
    add $4,%rsp
	  iretq
