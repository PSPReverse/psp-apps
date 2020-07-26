/** @file
 * PSP internal interfaces - Checkpoint API.
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
.section .text

/* bool PSPCheckPointSet(PSPCHCKPT *pChkPt) */
.globl PSPCheckPointSet;
PSPCheckPointSet:
    str r0,  [r0]
    str r1,  [r0, #4]
    str r2,  [r0, #8]
    str r3,  [r0, #12]
    str r4,  [r0, #16]
    str r5,  [r0, #20]
    str r6,  [r0, #24]
    str r7,  [r0, #28]
    str r8,  [r0, #32]
    str r9,  [r0, #36]
    str r10, [r0, #40]
    str r11, [r0, #44]
    str r12, [r0, #48]
    str sp,  [r0, #52]
    str lr,  [r0, #56]

    ldr r1, =_PSPCheckPointRestart;
    str r1, [r0, #60]
    mrs r1, cpsr
    str r1, [r0, #64]
    ldr r1, [r0, #4]
    mov r0, #1
    mov pc, lr

_PSPCheckPointRestart:
    /* r0 should be restored at that point already and point to the checkpoint state to use. */
    ldr r1,  [r0, #4]
    ldr r2,  [r0, #8]
    ldr r3,  [r0, #12]
    ldr r4,  [r0, #16]
    ldr r5,  [r0, #20]
    ldr r6,  [r0, #24]
    ldr r7,  [r0, #28]
    ldr r8,  [r0, #32]
    ldr r9,  [r0, #36]
    ldr r10, [r0, #40]
    ldr r11, [r0, #44]
    ldr r12, [r0, #48]
    ldr sp,  [r0, #52]
    ldr lr,  [r0, #56]

    mov r0, #0
    mov pc, lr
.type PSPCheckPointSet, %function;

