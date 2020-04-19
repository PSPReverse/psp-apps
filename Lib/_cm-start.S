/** @file
 * Code Module support - startup code which comes first.
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
 * This is all ugly AF but in order two mix and match ARM with Thumb code
 * we need our entry to set everything up correctly because the calling code
 * assumes ARM mode (and can't know whether a code mocule contains Thumb code).
 */
.extern main
_start:
    push {lr}
    ldr lr, [sp, #-8] /* Relocate the last two arguments on the stack */
    push {lr}
    ldr lr, [sp, #-16]
    push {lr}
    blx main
    pop {lr}
    pop {lr}
    pop {lr}
    bx lr

