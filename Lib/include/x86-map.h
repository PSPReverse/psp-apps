/** @file
 * PSP internal interfaces - UART driver.
 */

/* Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
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

/**
 * x86 memory mapping slot.
 */
typedef struct PSPX86MAPPING
{
    /** The base X86 address being mapped (aligned to a 64MB boundary). */
    X86PADDR                PhysX86AddrBase;
    /** The memory type being used. */
    uint32_t                uMemType;
    /** Reference counter for this mapping, the mapping gets cleaned up if it reaches 0. */
    uint32_t                cRefs;
} PSPX86MAPPING;

/** Pointer to an x86 memory mapping slot. */
typedef PSPX86MAPPING *PPSPX86MAPPING;

/** Pointer to a const x86 memory mapping slot. */
typedef const PSPX86MAPPING *PCPSPX86MAPPING;

/**
 * Maps the given x86 physical address into the PSP address space.
 *
 * @returns Status code.
 * @param   PhysX86Addr             The x86 physical address to map.
 * @param   fMmio                   Flag whether this a MMIO address.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspX86PhysMap(X86PADDR PhysX86Addr, bool fMmio, void **ppv);

/**
 * Unmaps a previously mapped x86 physical address.
 *
 * @returns Status code.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubX86PhysMap().
 */
int pspX86PhysUnmapByPtr(void *pv);

/**
 * Stores a 32 bit value at the provided x86 address.
 *
 * @param   PhysX86Addr             x86 target address.
 * @param   u32Val                  Value to store at "PhysX86Addr".
 */
void pspX86MmioWriteU32(X86PADDR PhysX86Addr, uint32_t u32Val);

/**
 * Stores a 8 bit value at the provided x86 address.
 *
 * @param   PhysX86Addr             x86 target address.
 * @param   bVal                    Value to store at "PhysX86Addr".
 */
void pspX86MmioWriteU8(X86PADDR PhysX86Addr, uint8_t bVal);

#endif /* !__include_x86_map_h */
