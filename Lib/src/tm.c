/** @file
 * TM - Timkeeping manager API.
 */

/*
 * Copyright (C) 2013 Alexander Eichner <aeichner@aeichner.de>
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

#include <err.h>
#include <tm.h>
#include <log.h>

/**
 * Runs the timer slots.
 *
 * @returns nothing.
 * @param   pTm    The timekeeping manager.
 */
static void tmRunSlots(PTM pTm)
{
    uint32_t cMillies = TMGetMillies(pTm);

    for (unsigned i = 0; i < pTm->cTimerCallbacks; i++)
    {
        PTMCLBKSLOT pSlot = &pTm->aClbkSlots[i];

        if (   pSlot->cMilliesNext != TM_TIMER_CALLBACK_DISABLED
            && pSlot->cMilliesNext <= cMillies)
        {
            /* Stop timer first, callee has to enable it again if required. */
            pSlot->cMilliesNext = TM_TIMER_CALLBACK_DISABLED;
            pSlot->pfnTmClbk(pTm, pSlot, pSlot->pvUser);
        }
    }
}

int TMInit(PTM pTm)
{
    pTm->cMicroSec = 0;
    pTm->cTimerCallbacks = 0;

    for (unsigned i = 0; i < TM_TIMER_CALLBACK_SLOT_COUNT; i++)
    {
        PTMCLBKSLOT pSlot = &pTm->aClbkSlots[i];

        pSlot->idxSlot      = i;
        pSlot->cMilliesNext = TM_TIMER_CALLBACK_DISABLED;
        pSlot->pfnTmClbk    = NULL;
        pSlot->pvUser       = NULL;
    }

    return INF_SUCCESS;
}

void TMTick(PTM pTm)
{
    pTm->cMicroSec++;

    /* Check for expired timers. */
    tmRunSlots(pTm);
}

void TMTickMultiple(PTM pTm, uint64_t cMicroSecs)
{
    pTm->cMicroSec += cMicroSecs;

    /* Check for expired timers. */
    tmRunSlots(pTm);
}

uint64_t TMGetMicros(PTM pTm)
{
    if (!pTm)
        return 0;

    return pTm->cMicroSec;
}

uint32_t TMGetMillies(PTM pTm)
{
    if (!pTm)
        return 0;

    return (uint32_t)(pTm->cMicroSec / 1000);
}

void TMDelayMillies(PTM pTm, uint32_t cMillies)
{
    uint32_t msStart, msEnd;

    msStart = TMGetMillies(pTm);
    msEnd = msStart + cMillies;

    if (msStart < msEnd)
    {
        while ( (TMGetMillies(pTm) >= msStart) && (TMGetMillies(pTm) < msEnd));
    }
    else
    {
        while ( (TMGetMillies(pTm) >= msStart) || (TMGetMillies(pTm) < msEnd));
    }
}

void TMDelayMicros(PTM pTm, uint64_t cMicros)
{
    uint64_t tsStart, tsEnd;

    tsStart = TMGetMicros(pTm);
    tsEnd = tsStart + cMicros;

    if (tsStart < tsEnd)
    {
        while ( (TMGetMicros(pTm) >= tsStart) && (TMGetMicros(pTm) < tsEnd));
    }
    else
    {
        while ( (TMGetMicros(pTm) >= tsStart) || (TMGetMicros(pTm) < tsEnd));
    }
}

int TMCallbackRegister(PTM pTm, PFNTMCLBK pfnTmClbk, void *pvUser, PTMCLBKSLOT *ppTmSlot)
{
    if (pTm->cTimerCallbacks == TM_TIMER_CALLBACK_SLOT_COUNT)
        return ERR_TM_OUT_OF_SLOTS;

    PTMCLBKSLOT pSlot = &pTm->aClbkSlots[pTm->cTimerCallbacks];

    pTm->cTimerCallbacks++;
    pSlot->pfnTmClbk = pfnTmClbk;
    pSlot->pvUser    = pvUser;
    *ppTmSlot = pSlot;

    return INF_SUCCESS;
}

int TMCallbackDeregister(PTM pTm, PTMCLBKSLOT pTmSlot)
{
    return ERR_NOT_IMPLEMENTED;
}

int TMCallbackSetExpirationAbsolute(PTM pTm, PTMCLBKSLOT pTmSlot, uint32_t cMillies)
{
    pTmSlot->cMilliesNext = cMillies;
    return INF_SUCCESS;
}

int TMCallbackSetExpirationRelative(PTM pTm, PTMCLBKSLOT pTmSlot, uint32_t cMillies)
{
    pTmSlot->cMilliesNext = TMGetMillies(pTm) + cMillies;
    return INF_SUCCESS;
}

int TMCallbackStop(PTM pTm, PTMCLBKSLOT pTmSlot)
{
    pTmSlot->cMilliesNext = TM_TIMER_CALLBACK_DISABLED;
    return INF_SUCCESS;
}

