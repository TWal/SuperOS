    .global _start
_start:
    ## startfile for Super OS usermode
    xchg %bx,%bx
    push %rdi
    push %rsi
    call __cinit                  # given by libc
    call __cppinit                # c++ constructors
    pop %rsi
    pop %rdi
    call main
    mov %rax,%rdi
    mov $58,%rax                # exit code
    syscall                     # exit

    ## Starting Convention.
    ## Stack Empty
    ## argc in %rdi
    ## argv in %rsi
