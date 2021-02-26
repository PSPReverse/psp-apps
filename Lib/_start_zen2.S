.extern SCRATCH_STACK_TOP
.extern BSS_START
.extern BSS_END
.extern main
.extern memset
_start:
  mov r4, r0         /* Save the arguments. */
  mov r5, r1
  mov r6, r2
  mov r7, r3
  ldr sp, =SCRATCH_STACK_TOP
  mov r0, $0x4b000   /* Start address of main stack */
  mov r1, $0x4d000   /* End address of main stack */
  mov r2, sp         /* Address where to return the mapped memory area */
  svc $1             /* Call map stack service call. */
  ldr r0, [sp, #0x0] /* Load stack address from scratch stack. */
  mov sp, r0         /* Switch to the new stack. */
  mov r0, r4         /* Restore arguments. */
  mov r1, r5
  mov r2, r6
  mov r3, r7
  blx main

  /* Exit gracefully */
  svc $0x0

 /* vim: ft=gas :
 */ 
