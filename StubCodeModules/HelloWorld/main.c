/** @file
 * PSP code module - Hello World.
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
#include <types.h>
#include <cdefs.h>

#include <psp-stub/cm-if.h>

uint32_t main(PCCMIF pCmIf, uint32_t u32Arg0, uint32_t u32Arg1, uint32_t u32Arg2, uint32_t u32Arg3)
{
    return pCmIf->pfnOutBufWrite(pCmIf, 0 /*idOutBuf*/, "Hello World!\n", sizeof("Hello World!\n") - 1, NULL /*pcbWritten*/);
}
