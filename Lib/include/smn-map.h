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

#ifndef __include_smn_map_h
#define __include_smn_map_h

#include <types.h>

/**
 * SMN mapping slot.
 */
typedef struct PSPSMNMAPPING
{
    /** Base SMN address being mapped (aligned to a 1MB boundary). */
    SMNADDR                 SmnAddrBase;
    /* Reference counter for this mapping, the mapping gets cleand up if it reaches 0. */
    uint32_t                cRefs;
} PSPSMNMAPPING;

/** Pointer to a SMN mapping slot. */
typedef PSPSMNMAPPING *PPSPSMNMAPPING;

/** Pointer to a const SMN mapping slot. */
typedef const PSPSMNMAPPING *PCPSPSMNMAPPING;

/**
 * Maps the given SMN address into the PSP address space.
 *
 * @returns Status code.
 * @param   SmnAddr                 The SMN address to map.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspSmnMap(SMNADDR SmnAddr, void **ppv);

/**
 * Unmaps a previously mapped SMN address.
 *
 * @returns Status code.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubSmnMap().
 */
int pspSmnUnmapByPtr(void *pv);

/**
 * Stores a 32 bit value at the provided SMN address.
 *
 * @param   SmnAddr                 SMN target address.
 * @param   u32Val                  Value to store at "SmnAddr".
 */
void pspSmnWrU32(SMNADDR SmnAddr, uint32_t u32Val);

#endif /* !__include_smn_map_h */
