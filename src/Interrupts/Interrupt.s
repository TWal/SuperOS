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
    testb $2,(%rbp,%rbx)
    cmovz %rcx,%rax
    pop %rcx
    pop %rdx
    pop %rdi
	  pop %rsi
    testb $4,(%rbp,%rbx)
	  jnz errEnd
	  pop %rbp
    pop %rbx
    iretq

errEnd:
	  pop %rbp
	  pop %rbx
    add $8,%rsp
	  iretq
