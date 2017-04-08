    .global _start
_start:
    ## startfile for Super OS usermode

    call __cinit                  # given by libc
    call __cppinit                # c++ constructors
    call main                   # TODO : send argv and argc
    mov %rax,%rdi
    mov $60,%rax                # exit code
    syscall                     # exit

    ## Starting Convention.
    ## Stack Empty
    ## argc in %rdi
    ## argv in %rsi
    ## current brk in %rcx
