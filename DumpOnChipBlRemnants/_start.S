.extern main
_start:
  ldr r0, =0x35800
  ldr r1, =0x40000
  ldr r2, =0x01aab000

_loop_head:
  cmp r0, r1
  beq _exit
  ldr r3, [r0]
  str r3, [r2]
  add r0, r0, #4
  add r2, r2, #4
  b _loop_head

_exit:
  mov r0, #0

  /* Exit gracefully */
  svc $0x0

 /* vim: ft=gas :
 */ 
