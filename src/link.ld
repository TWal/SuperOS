ENTRY(kmain)


SECTIONS {
    . = . + 0xC0000000;

    .text ALIGN (0x1000) : AT(ADDR(.text)-0xC0000000)
    {
        *(.text)
        _text_end = .;
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata)-0xC0000000)
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data)-0xC0000000)
    {
        *(.data)
    }

    .bss ALIGN (0x1000) : AT(ADDR(.bss)-0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }
    .init_array ALIGN (0x1000) : AT(ADDR(.init_array)-0xC0000000)
    {
    __init_array_start = .;
    *(.init_array)
    __init_array_end = .;
    }

    kernel_code_end = . - 0xC0000000;
}
