ENTRY(loader)

SECTIONS {
    . = 0x00100000;

    .text ALIGN (0x1000):
    {
        *(.text)
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata*)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)


    }

    .bss ALIGN (0x1000) :
    {
        *(COMMON)
        *(.bss)
    }
    .init_array ALIGN (0x1000) :
    {
    __init_array_start = .;
    *(.init_array)
    __init_array_end = .;
    }
}
