    .global _exit
_exit:
    mov $58, %eax
    syscall
    ret
    
    .global _texit
_texit:
	mov $60, %eax
	syscall
	ret
