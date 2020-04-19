MEMORY
{
    RAM : ORIGIN = 0x10000, LENGTH = 0x2f000 /* Code module area from 0x10000 right to the start of the boot ROM servie page */
}


SECTIONS
{
    . = 0x10000;

    .text ALIGN(0x10):
    {
        *(.text);
        *(.text*);
    } > RAM

    .rodata ALIGN(0x10):
    {
        *(.rodata)
        *(.rodata*)
    } > RAM

    .data ALIGN(0x10):
    {
        *(.data)
        *(.data*)
    } > RAM

    .bss ALIGN(0x10):
    {
        BSS_START = .;
        *(.bss)
        *(.bss*)
        BSS_END = .;
    } > RAM
}

