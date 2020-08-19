/** @file
 * PSP internal interfaces - UART driver.
 */

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
 * Copyright (C) 2020 Robert Buhren <robert.buhren@sect.tu-berlin.de>
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
#ifndef __include_x86_map_h
#define __include_x86_map_h

#include <types.h>

int pspX86PhysMap(X86PADDR PhysX86Addr, bool fMmio, void **ppv);
int pspX86PhysUnmapByPtr(void *pv);
void pspX86MmioWriteU32(X86PADDR PhysX86Addr, uint32_t u32Val);
void pspX86MmioWriteU8(X86PADDR PhysX86Addr, uint8_t bVal);
void pspX86MapInit(void);

#endif /* !__include_x86_map_h */

