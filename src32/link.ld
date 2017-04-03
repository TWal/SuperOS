ENTRY(loader)



SECTIONS {

    . = 0x00100000;

    .text ALIGN (0x1000) :
    {
        *(.text)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .data.pages ALIGN (0x1000) :
    {
        *(.data.pages)
    }


    .rodata ALIGN (0x1000) :
    {
        *(.rodata)
    }

    .bss ALIGN (0x1000) :
    {
        *(.bss)
    }

    .init_array ALIGN (0x1000) :
    {
    __init_array_start = .;
    *(.init_array)
    __init_array_end = .;
    }

    loader_code_end = .;
}
