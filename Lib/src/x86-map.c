/** @file Basic x86 mapping driver.
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
#include <types.h>
#include <cdefs.h>
#include <x86-map.h>
#include <err.h>
#include <common/status.h>
#include <string.h>

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

/** x86 mapping bookkeeping data. */
static PSPX86MAPPING        aX86MapSlots[15];

/** Pointer to an x86 memory mapping slot. */
typedef PSPX86MAPPING *PPSPX86MAPPING;
/** Pointer to a const x86 memory mapping slot. */
typedef const PSPX86MAPPING *PCPSPX86MAPPING;

/**
 * Maps the given x86 physical address into the PSP address space.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   PhysX86Addr             The x86 physical address to map.
 * @param   fMmio                   Flag whether this a MMIO address.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspX86PhysMap(X86PADDR PhysX86Addr, bool fMmio, void **ppv)
{
    int rc = INF_SUCCESS;
    uint32_t uMemType = fMmio ? 0x6 : 0x4;

    /* Split physical address into 64MB aligned base and offset. */
    X86PADDR PhysX86AddrBase = (PhysX86Addr & ~(_64M - 1));
    uint32_t offStart = PhysX86Addr - PhysX86AddrBase;

    PPSPX86MAPPING pMapping = NULL;
    uint32_t idxSlot = 0;
    for (uint32_t i = fMmio ? 8 : 0; i < ELEMENTS(aX86MapSlots); i++)
    {
        if (   (   aX86MapSlots[i].PhysX86AddrBase == NIL_X86PADDR
                && aX86MapSlots[i].cRefs == 0)
            || (   aX86MapSlots[i].PhysX86AddrBase == PhysX86AddrBase
                && aX86MapSlots[i].uMemType == uMemType))
        {
            pMapping = &aX86MapSlots[i];
            idxSlot = i;
            break;
        }
    }

    if (pMapping)
    {
        if (pMapping->PhysX86AddrBase == NIL_X86PADDR)
        {
            /* Set up the mapping. */
            pMapping->uMemType         = uMemType;
            pMapping->PhysX86AddrBase  = PhysX86AddrBase;

            /* Program base address. */
            PSPADDR PspAddrSlotBase = 0x03230000 + idxSlot * 4 * sizeof(uint32_t);
            *(volatile uint32_t *)PspAddrSlotBase        = ((PhysX86AddrBase >> 32) << 6) | ((PhysX86AddrBase >> 26) & 0x3f);
            *(volatile uint32_t *)(PspAddrSlotBase + 4)  = 0x12; /* Unknown but fixed value. */
            *(volatile uint32_t *)(PspAddrSlotBase + 8)  = uMemType;
            *(volatile uint32_t *)(PspAddrSlotBase + 12) = uMemType;
            *(volatile uint32_t *)(0x032303e0 + idxSlot * sizeof(uint32_t)) = 0xffffffff;
            *(volatile uint32_t *)(0x032304d8 + idxSlot * sizeof(uint32_t)) = 0xc0000000;
            *(volatile uint32_t *)0x32305ec = 0x3333;
        }

        asm volatile("dsb #0xf\nisb #0xf\n": : :"memory");
        pMapping->cRefs++;
        *ppv = (void *)(0x04000000 + idxSlot * _64M + offStart);
    }
    else
        rc = ERR_INVALID_STATE;

    return rc;
}


/**
 * Unmaps a previously mapped x86 physical address.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubX86PhysMap().
 */
int pspX86PhysUnmapByPtr(void *pv)
{
    int rc = INF_SUCCESS;
    uintptr_t PspAddrMapBase = ((uintptr_t)pv) & ~(_64M - 1);
    PspAddrMapBase -= 0x04000000;

    uint32_t idxSlot = PspAddrMapBase / _64M;
    if (   idxSlot < ELEMENTS(aX86MapSlots)
        && PspAddrMapBase % _64M == 0)
    {
        PPSPX86MAPPING pMapping = &aX86MapSlots[idxSlot];

        asm volatile("dsb #0xf\nisb #0xf\n": : :"memory");
        if (pMapping->cRefs > 0)
        {
            pMapping->cRefs--;

            /* Clear out the mapping if there is no reference held. */
            if (!pMapping->cRefs)
            {
                pMapping->uMemType        = 0;
                pMapping->PhysX86AddrBase = NIL_X86PADDR;

                PSPADDR PspAddrSlotBase = 0x03230000 + idxSlot * 4 * sizeof(uint32_t);
                *(volatile uint32_t *)PspAddrSlotBase        = 0;
                *(volatile uint32_t *)(PspAddrSlotBase + 4)  = 0; /* Unknown but fixed value. */
                *(volatile uint32_t *)(PspAddrSlotBase + 8)  = 0;
                *(volatile uint32_t *)(PspAddrSlotBase + 12) = 0;
                *(volatile uint32_t *)(0x032303e0 + idxSlot * sizeof(uint32_t)) = 0xffffffff;
                *(volatile uint32_t *)(0x032304d8 + idxSlot * sizeof(uint32_t)) = 0;
            }
        }
        else
            rc = ERR_INVALID_PARAMETER;
    }
    else
        rc = ERR_INVALID_PARAMETER;

    return rc;
}

void pspX86MmioWriteU32(X86PADDR PhysX86Addr, uint32_t u32Val)
{
    volatile uint32_t *pu32 = NULL;
    int rc = pspX86PhysMap(PhysX86Addr, true /*fMmio*/, (void **)&pu32);
    if (STS_SUCCESS(rc))
    {
        *pu32 = u32Val;
        pspX86PhysUnmapByPtr((void *)pu32);
    }
}

void pspX86MmioWriteU8(X86PADDR PhysX86Addr, uint8_t bVal)
{
    volatile uint8_t *pb = NULL;
    int rc = pspX86PhysMap(PhysX86Addr, true /*fMmio*/, (void **)&pb);
    if (STS_SUCCESS(rc))
    {
        *pb = bVal;
        pspX86PhysUnmapByPtr((void *)pb);
    }
}


void pspX86MapInit(void) {
    memset(aX86MapSlots, 0, sizeof(aX86MapSlots));
    for (uint32_t i = 0; i < ELEMENTS(aX86MapSlots); i++)
        aX86MapSlots[i].PhysX86AddrBase = NIL_X86PADDR;
}
