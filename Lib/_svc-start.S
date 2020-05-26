/** @file
 * PSP serial stub - SVC startup code.
 */

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
.extern SCRATCH_STACK_TOP_SVC
.extern SCRATCH_STACK_TOP_UNDEF
.extern SCRATCH_STACK_TOP_ABORT
.extern SCRATCH_STACK_TOP_IRQ
.extern SCRATCH_STACK_TOP_FIQ
.extern BSS_START
.extern BSS_END
.extern main
.extern memset
.extern ExcpUndefInsn
.extern ExcpSwi
.extern ExcpPrefAbrt
.extern ExcpDataAbrt
.extern ExcpIrq
.extern ExcpFiq


/**
 * Macro generating a an exception handler entry point.
 *
 * @param a_Name                Name of the entry point
 * @param a_FnCall              The function to call.
 */
.macro EXCP_ENTRY_POINT_GEN a_Name a_FnCall
\a_Name:
    stmfd sp!, {lr}         /* Save LR */
    mrs lr, spsr            /* Save SPSR */
    stmfd sp!, {lr}
    stmfd sp!, {r0-r12}     /* Save register set. */
    mov r0, sp
    blx \a_FnCall
    ldmfd sp!, {r0-r12}     /* Restore register set. */
    ldmfd sp!, {lr}         /* Restore SPSR */
    msr spsr, lr
    ldmfd sp!, {lr}         /* Restore LR */
    subs pc, lr, #0         /* Switch back to where we came from, lr gets adjusted in the respective exception C handlers */
.endm

_g_aExcpVectors:
    ldr pc, =_start
    ldr pc, =_undef_insn
    ldr pc, =_swi
    ldr pc, =_prefetch_abort
    ldr pc, =_data_abort
    nop
    ldr pc, =_irq
    ldr pc, =_fiq

_start:
    mrc p15, #0x0, r1, cr1, cr0, #0x0       /* Read MMU control register and clear V bit to set the exception vector table address to the low range */
    bic r1, r1, #0x2000
    mcr p15, #0x0, r1, cr1, cr0, #0x0
    ldr r1, =_g_aExcpVectors                /* Set the exception vector table address. */
    mcr p15, #0x0, r1, cr12, cr0, #0x0
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
    ldr r0, =BSS_START
    mov r1, #0
    ldr r2, =BSS_END
    sub r2, r2, r0
    blx memset                        /* Clear BSS section. */
    blx main

/* Generate default exception handlers. */
EXCP_ENTRY_POINT_GEN _undef_insn     ExcpUndefInsn
EXCP_ENTRY_POINT_GEN _prefetch_abort ExcpPrefAbrt
EXCP_ENTRY_POINT_GEN _data_abort     ExcpDataAbrt
EXCP_ENTRY_POINT_GEN _irq            ExcpIrq

_swi:
    blx ExcpSwi
    b fault

_fiq:
    blx ExcpFiq
    b fault

fault:
    b fault

