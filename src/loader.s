.global loader

.equ MAGIC_NUMBER, 0x1BADB002
.equ FLAGS, 0x0
.equ CHECKSUM, -MAGIC_NUMBER
# MAGICNUMBER + FLAGS + CHECKSUM = 0

.text
.int MAGIC_NUMBER, FLAGS, CHECKSUM

loader:
    mov $0xCAFEBABE, %eax
loop:
    jmp loop
