/** @file SMN mapping driver.
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
#include <smn-map.h>
#include <err.h>
#include <string.h>
#include <common/status.h>

/** SMN mapping bookeeping data. */
static PSPSMNMAPPING               g_aSmnMapSlots[32];
static uint32_t                    g_u32SmnInitialized = 0;

static void pspSmnMapInit(void) {
    memset(g_aSmnMapSlots, 0, sizeof(g_aSmnMapSlots));
    g_u32SmnInitialized = 1;
}

int pspSmnMap(SMNADDR SmnAddr, void **ppv)
{
    int rc = INF_SUCCESS;

    if (!g_u32SmnInitialized)
        pspSmnMapInit();

    /* Split physical address into 1MB aligned base and offset. */
    SMNADDR  SmnAddrBase = (SmnAddr & ~(_1M - 1));
    uint32_t offStart = SmnAddr - SmnAddrBase;

    PPSPSMNMAPPING pMapping = NULL;
    uint32_t idxSlot = 0;
    for (uint32_t i = 0; i < ELEMENTS(g_aSmnMapSlots); i++)
    {
        if (   (   g_aSmnMapSlots[i].SmnAddrBase == 0
                && g_aSmnMapSlots[i].cRefs == 0)
            || g_aSmnMapSlots[i].SmnAddrBase == SmnAddrBase)
        {
            pMapping = &g_aSmnMapSlots[i];
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

    return rc;
}

int pspSmnUnmapByPtr(void *pv)
{
    int rc = INF_SUCCESS;

    if (!g_u32SmnInitialized)
        return ERR_INVALID_STATE;

    uintptr_t PspAddrMapBase = ((uintptr_t)pv) & ~(_1M - 1);
    PspAddrMapBase -= 0x01000000;

    uint32_t idxSlot = PspAddrMapBase / _1M;
    if (   idxSlot < ELEMENTS(g_aSmnMapSlots)
        && PspAddrMapBase % _1M == 0)
    {
        PPSPSMNMAPPING pMapping = &g_aSmnMapSlots[idxSlot];

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
