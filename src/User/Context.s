    .global testLaunchCPL3
testLaunchCPL3:
    xchg %bx,%bx
    xor %rax,%rax
    pushq $0x23
    push %rsp
    add $8,(%rsp)
    pushfq
    pushq $0x2B
    push $testCPL3
    iretq

testCPL3:
    xchg %bx,%bx
    mov $42,%rax
    syscall
    xchg %bx,%bx
    mov $42,%rax
    int $0x80
    xchg %bx,%bx
    mov $43, %rax
    syscall

