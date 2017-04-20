    .global _start
_start:
    ## startfile for Super OS usermode
    xchg %bx,%bx

    call __cinit                  # given by libc
    call __cppinit                # c++ constructors
    call main                   # TODO : send argv and argc
    mov %rax,%rdi
    mov $58,%rax                # exit code
    syscall                     # exit

    ## Starting Convention.
    ## Stack Empty
    ## argc in %rdi
    ## argv in %rsi
