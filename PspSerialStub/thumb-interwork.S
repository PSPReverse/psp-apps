/** @file
 * PSP serial stub - Thumb interworking code.
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
 * This is ugly AF but in order to support ARM and thumb code modules there
 * doesn't seem to be a better way as gcc will always emit a ""ldr pc, ..." which
 * will not return the previous instruction set state.
 */

/**
 * Macro generating a trampoline for functions with up to 4 arguments not requiring
 * repositioning some arguments on the stack.
 *
 * @param a_Name                Name of the function generate the trampoline for.
 */
.macro FN_INTERWORK_1_2_3_4_ARG a_Name
.globl \a_Name\()Asm
\a_Name\()Asm:
    push {lr}
    blx \a_Name
    pop {lr}
    bx lr
.type \a_Name\()Asm, %function;
.endm


/**
 * Macro generating a trampoline for functions with up 5 arguments which requires
 * repositioning one argument on the stack.
 *
 * @param a_Name                Name of the function generate the trampoline for.
 */
.macro FN_INTERWORK_5_ARG a_Name
.globl \a_Name\()Asm
\a_Name\()Asm:
    push {lr}
    ldr lr, [sp, #4] /* Rearrange the last argument on the stack as the lr push has changed the layout. */
    push {lr}
    blx \a_Name
    pop {lr}
    pop {lr}
    bx lr
.type \a_Name\()Asm, %function;
.endm

/* External references to methods we are creating tramponlines for. */
.extern pspStubCmIfInBufPeek
.extern pspStubCmIfInBufPoll
.extern pspStubCmIfInBufRead
.extern pspStubCmIfOutBufWrite
.extern pspStubCmIfDelayMs
.extern pspStubCmIfTsGetMilli

/* Generates the trampolines */
FN_INTERWORK_1_2_3_4_ARG pspStubCmIfInBufPeek
FN_INTERWORK_1_2_3_4_ARG pspStubCmIfInBufPoll
FN_INTERWORK_5_ARG pspStubCmIfInBufRead
FN_INTERWORK_5_ARG pspStubCmIfOutBufWrite
FN_INTERWORK_1_2_3_4_ARG pspStubCmIfDelayMs
FN_INTERWORK_1_2_3_4_ARG pspStubCmIfTsGetMilli

