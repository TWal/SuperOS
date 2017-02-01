.bss
  .global intIDT
intIDT:
  .space 4*256

  .global doReturn
doReturn:
  .space 256



.text
  .global geneInt
geneInt:
  push %ecx
  push %edx
  push %esi
  push %ebp
  push %edi
  push %ebx
  push %eax
  call *intIDT(%edi)
  mov 8(%esp),%edi
  pop %ecx
  testb $1,doReturn(%edi)
  cmovz %ecx,%eax
  pop %ebx
  pop %edi
  pop %ebp
  pop %esi
  pop %edx
  pop %ecx
  pop %edi
  iret
