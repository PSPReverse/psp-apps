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

