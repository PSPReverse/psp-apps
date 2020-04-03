.extern SCRATCH_STACK_TOP_SVC
.extern SCRATCH_STACK_TOP_UNDEF
.extern SCRATCH_STACK_TOP_ABORT
.extern SCRATCH_STACK_TOP_IRQ
.extern SCRATCH_STACK_TOP_FIQ
.extern main
.extern ExcpUndefInsn
.extern ExcpSwi
.extern ExcpPrefAbrt
.extern ExcpDataAbrt
.extern ExcpIrq
.extern ExcpFiq

ldr pc, =_start
ldr pc, =_undef_insn
ldr pc, =_swi
ldr pc, =_prefetch_abort
ldr pc, =_data_abort
nop
ldr pc, =_irq
ldr pc, =_fiq

_start:
  msr cpsr, #0xd2
  ldr sp, =SCRATCH_STACK_TOP_IRQ
  msr cpsr, #0xd1
  ldr sp, =SCRATCH_STACK_TOP_FIQ
  msr cpsr, #0xd7
  ldr sp, =SCRATCH_STACK_TOP_ABORT
  msr cpsr, #0xdb
  ldr sp, =SCRATCH_STACK_TOP_UNDEF
  msr cpsr, #0x53
  ldr sp, =SCRATCH_STACK_TOP_SVC
  blx main

_undef_insn:
  blx ExcpUndefInsn
  b fault

_swi:
  blx ExcpSwi
  b fault

_prefetch_abort:
  blx ExcpPrefAbrt
  b fault

_data_abort:
  blx ExcpDataAbrt
  b fault

_irq:
  blx ExcpIrq
  b fault

_fiq:
  blx ExcpFiq
  b fault

fault:
    b fault

 /* vim: ft=gas :
 */ 
