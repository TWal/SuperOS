.bss
    .global intIDT
intIDT:
    .space 4*256

    .global params
params:
    .space 256



.text
    .global geneInt
geneInt: # by default interruption on user stack.
    push %ebp
    push %esi
    push %edi
    push %edx
    push %ecx
    push %ebx
    push %eax
  	push %esp
    call *intIDT(%edi)
    mov 20(%esp),%edi
    shrl $2,%edi
    add $4,%esp
    pop %ecx
    testb $2,params(%edi)
    cmovz %ecx,%eax
    pop %ebx
    pop %ecx
    pop %edx
    testb $4,params(%edi)
	  jnz errEnd
    pop %edi
    pop %esi
    pop %ebp
    pop %edi
    iret

errEnd:
    pop %edi
    pop %esi
	  pop %ebp
	  pop %edi
    add $4,%esp
	  iret
