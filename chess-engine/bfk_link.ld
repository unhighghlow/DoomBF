ENTRY(_start)

MEMORY
{
    IMEM (x)  : ORIGIN = 0x10000000, LENGTH = 0xefffffff
    DMEM (rw)  : ORIGIN = 0x00000000, LENGTH = 0x0f000000
}

SECTIONS
{
    .text :
    {
        *(.text.init)
        *(.text*)
    } > IMEM

    .rodata :
    {
        *(.rodata*)
    } > DMEM

    .data :
    {
        *(.data*)
    } > DMEM

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    } > DMEM
}
