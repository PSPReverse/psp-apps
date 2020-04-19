
SECTIONS
{
  . = 0x15100;
  .text ALIGN(0x10): { *(.text) }
  .data ALIGN(0x10) : { *(.data) }
  BSS_START = .;
  .bss ALIGN(0x10) : { *(.bss) }
  BSS_END = .;

  SCRATCH_STACK_BASE = .;
  . += 0x2000;
  SCRATCH_STACK_TOP = .;
}
