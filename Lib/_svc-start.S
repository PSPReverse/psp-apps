.extern SCRATCH_STACK_TOP
.extern main
_start:
  ldr sp, =SCRATCH_STACK_TOP
  blx main

fault:
    b fault

 /* vim: ft=gas :
 */ 
