MEMORY
{
    RAM : ORIGIN = 0x0, LENGTH = 64K
}


SECTIONS
{
    . = 0x100;

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

    .stack ALIGN(0x10):
    {
        SCRATCH_STACK_BASE_SVC = .;
        . += 0x2000;
        SCRATCH_STACK_TOP_SVC = .;

        SCRATCH_STACK_BASE_UNDEF = .;
        . += 0x200;
        SCRATCH_STACK_TOP_UNDEF = .;

        SCRATCH_STACK_BASE_ABORT = .;
        . += 0x200;
        SCRATCH_STACK_TOP_ABORT = .;

        SCRATCH_STACK_BASE_IRQ = .;
        . += 0x200;
        SCRATCH_STACK_TOP_IRQ = .;

        SCRATCH_STACK_BASE_FIQ = .;
        . += 0x200;
        SCRATCH_STACK_TOP_FIQ = .;
    } > RAM

    _ram_end = ORIGIN(RAM) + LENGTH(RAM);
}

