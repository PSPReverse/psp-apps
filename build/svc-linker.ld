
SECTIONS
{
  . = 0x100;
  .text ALIGN(0x10): { *(.text) }
  .data ALIGN(0x10) : { *(.data) }
  .bss ALIGN(0x10) : { *(.bss) }

  SCRATCH_STACK_BASE = .;
  . += 0x2000;
  SCRATCH_STACK_TOP = .;
}
