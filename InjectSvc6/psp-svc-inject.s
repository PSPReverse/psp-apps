
.section .text
.extern svc_inject_handler
.extern svc_app_cleanup_inject_handler

/*
 * Wrapper for main syscall handler of the PSP firmware
 */
_svc_entry:
    stmfd sp!, { r0, r1, lr }
    sub sp, #4
    mov r2, sp
    bl svc_inject_handler
    mov r2, r0
    cmp r2, #0
    bne _svc_not_handled
    ldr r0, [sp]                /* Adjust the return value. */
    add sp, #4
    ldmia sp!, { r0, r1, lr }
    mov pc, lr
_svc_not_handled:
    add sp, #4
    ldmia sp!, { r0, r1, lr }
    ldr r2, =_svc_orig_addr
    ldr r2, [r2]
    bx r2

/*
 * Wrapper for the cleanup userspace app handler of the PSP firmware
 */
/*
_svc_cleanup_entry:
    stmfd sp!, { r0, r1, lr }
    bl svc_app_cleanup_inject_handler
    ldmia sp!, { r0, r1, lr }
    ldr r2, =_svc_app_cleanup_orig_addr
    ldr r2, [r2]
    bx r2
*/

_svc_orig_addr:
    .word 0x9519
/*_svc_app_cleanup_orig_addr:
    .word 0x9e59 */
 /* vim: ft=gas :
 */ 
