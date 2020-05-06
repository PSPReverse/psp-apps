/** @file
 * PSP serial stub - Utility assembly routines.
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

/**
 * Writes to a co processor register
 *
 * @returns nothing.
 * @param   r0                      The value to write.
 */
.globl pspSerialStubCoProcWriteAsm
pspSerialStubCoProcWriteAsm:
    mcr p15, 0, r0, cr0, cr0, 0 /* This will get overwritten with the appropriate arguments. */
    bx lr
.type pspSerialStubCoProcWriteAsm, %function;


/**
 * Reads from a co processor register
 *
 * @returns The value read.
 */
.globl pspSerialStubCoProcReadAsm
pspSerialStubCoProcReadAsm:
    mrc p15, 0, r0, cr0, cr0, 0 /* This will get overwritten with the appropriate arguments. */
    bx lr
.type pspSerialStubCoProcReadAsm, %function;


/**
 * Branches to a given address using the given register set.
 *
 * @returns The value read.
 */
.globl pspStubBranchToAsm
pspStubBranchToAsm:
    mov lr, r0
    mov r12, r1
    ldmia r12!, {r0-r11}
    ldr r12, [r12]
    bx lr                   /* This branches to the destination, bye bye world it was really nice here... */
.type pspStubBranchToAsm, %function;


