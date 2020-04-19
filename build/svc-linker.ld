
SECTIONS
{
  . = 0x100;
  .text ALIGN(0x10): { *(.text) }
  .data ALIGN(0x10) : { *(.data) }
  BSS_START = .;
  .bss ALIGN(0x10) : { *(.bss) }
  BSS_END = .;

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
}
