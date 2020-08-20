/** @file Basic smn mapping driver.
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
#include <string.h>
#include <common/status.h>

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

static PSPSMNMAPPING               aSmnMapSlots[32];

/**
 * Maps the given SMN address into the PSP address space.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   SmnAddr                 The SMN address to map.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspSmnMap(SMNADDR SmnAddr, void **ppv)
{
    int rc = INF_SUCCESS;

    /* Split physical address into 1MB aligned base and offset. */
    SMNADDR  SmnAddrBase = (SmnAddr & ~(_1M - 1));
    uint32_t offStart = SmnAddr - SmnAddrBase;

    PPSPSMNMAPPING pMapping = NULL;
    uint32_t idxSlot = 0;
    for (uint32_t i = 0; i < ELEMENTS(aSmnMapSlots); i++)
    {
        if (   (   aSmnMapSlots[i].SmnAddrBase == 0
                && aSmnMapSlots[i].cRefs == 0)
            || aSmnMapSlots[i].SmnAddrBase == SmnAddrBase)
        {
            pMapping = &aSmnMapSlots[i];
            idxSlot = i;
            break;
        }
    }

    if (pMapping)
    {
        if (pMapping->SmnAddrBase == 0)
        {
            /* Set up the mapping. */
            pMapping->SmnAddrBase = SmnAddrBase;

            /* Program base address. */
            PSPADDR PspAddrSlotBase = 0x03220000 + (idxSlot / 2) * sizeof(uint32_t);
            uint32_t u32RegSmnMapCtrl = *(volatile uint32_t *)PspAddrSlotBase;
            if (idxSlot & 0x1)
                u32RegSmnMapCtrl |= ((SmnAddrBase >> 20) << 16);
            else
                u32RegSmnMapCtrl |= SmnAddrBase >> 20;
            *(volatile uint32_t *)PspAddrSlotBase = u32RegSmnMapCtrl;
        }

        pMapping->cRefs++;
        *ppv = (void *)(0x01000000 + idxSlot * _1M + offStart);
    }
    else
        rc = ERR_INVALID_STATE;

    //LogRel("pspStubSmnUnmapByPtr: SmnAddr=%#x -> rc=%d,pv=%p\n", SmnAddr, rc, *ppv);
    return rc;
}


/**
 * Unmaps a previously mapped SMN address.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubSmnMap().
 */
int pspSmnUnmapByPtr(void *pv)
{
    int rc = INF_SUCCESS;
    uintptr_t PspAddrMapBase = ((uintptr_t)pv) & ~(_1M - 1);
    PspAddrMapBase -= 0x01000000;

    uint32_t idxSlot = PspAddrMapBase / _1M;
    if (   idxSlot < ELEMENTS(aSmnMapSlots)
        && PspAddrMapBase % _1M == 0)
    {
        PPSPSMNMAPPING pMapping = &aSmnMapSlots[idxSlot];

        if (pMapping->cRefs > 0)
        {
            pMapping->cRefs--;

            /* Clear out the mapping if there is no reference held. */
            if (!pMapping->cRefs)
            {
                pMapping->SmnAddrBase = 0;

                PSPADDR PspAddrSlotBase = 0x03220000 + (idxSlot / 2) * sizeof(uint32_t);
                uint32_t u32RegSmnMapCtrl = *(volatile uint32_t *)PspAddrSlotBase;
                if (idxSlot & 0x1)
                    u32RegSmnMapCtrl &= 0xffff;
                else
                    u32RegSmnMapCtrl &= 0xffff0000;
                *(volatile uint32_t *)PspAddrSlotBase = u32RegSmnMapCtrl;
            }
        }
        else
            rc = ERR_INVALID_PARAMETER;
    }
    else
        rc = ERR_INVALID_PARAMETER;

    /*LogRel("pspStubSmnUnmapByPtr: pv=%p -> rc=%d\n", pv, rc);*/
    return rc;
}

void pspSmnWrU32(SMNADDR SmnAddr, uint32_t u32Val)
{
    void *pvMap = NULL;
    int rc = pspSmnMap(SmnAddr, &pvMap);
    if (!rc)
    {
        *(volatile uint32_t*) pvMap = u32Val;
        pspSmnUnmapByPtr(pvMap);
    }
}

void pspSmnMapInit(void) {
    memset(aSmnMapSlots, 0, sizeof(aSmnMapSlots));
}
