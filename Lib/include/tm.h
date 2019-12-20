/** @file
 * Timekeeping manager.
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
#ifndef ___tm_h
#define ___tm_h

#include <types.h>

/** Number of slots for timer callbacks. */
#ifndef TM_TIMER_CALLBACK_SLOT_COUNT
# define TM_TIMER_CALLBACK_SLOT_COUNT 10
#endif

/** Special milliseconds timer value for a disabled callback. */
#define TM_TIMER_CALLBACK_DISABLED UINT32_MAX

/* A few required forward declarations. */

/** Pointer to a timer callback slot. */
typedef struct TMCLBKSLOT *PTMCLBKSLOT;
/** Pointer to a timekeeping manager instance. */
typedef struct TM *PTM;

/**
 * Timer callback.
 *
 * @returns nothing.
 * @param   pTm     The timekeeping manager this callback belongs to.
 * @param   pTmClbk The particular slot the callbak is registered in.
 * @param   pvUser  Opaque user data given during registration.
 */
typedef void FNTMCLBK(PTM pTm, PTMCLBKSLOT pTmClbk, void *pvUser);
/** Pointer to a timer callback. */
typedef FNTMCLBK *PFNTMCLBK;

/**
 * Timer callback slot.
 *
 * @note: Everything in this struct is private, don't access directly.
 */
typedef struct TMCLBKSLOT
{
    /** Index in the slot array. */
    uint32_t             idxSlot;
    /** When the callback should be fired again. */
    volatile uint32_t    cMilliesNext;
    /** Callback to execute. */
    PFNTMCLBK            pfnTmClbk;
    /** Opaque user data to pass in the callback. */
    void                *pvUser;
} TMCLBKSLOT;

/**
 * Timekeeping manager instance data.
 *
 * @note: Everything in this struct is private, don't access directly.
 */
typedef struct TM
{
    /** Current millisecond count. */
    volatile uint32_t cMillies;
    /** Number of timer callbacks registered. */
    volatile uint32_t cTimerCallbacks;
    /** Array of timer callbacks. */
    TMCLBKSLOT        aClbkSlots[TM_TIMER_CALLBACK_SLOT_COUNT];
} TM;

/**
 * Initialise the time keeping manager.
 * After this succeeded you can use TMTick() immediately.
 *
 * @returns Status code.
 * @param   pTm    The timekeeping manager to initialise.
 */
int TMInit(PTM pTm);

/**
 * Advances the clock of the timekeeping manager by one tick and
 * calls all expired callbacks.
 *
 * @note: This should be called every millisecond to get an accurate time source.
 *
 * @returns nothing.
 * @param   pTm    The timekeeping manager to use.
 */
void TMTick(PTM pTm);

/**
 * Return the current milliseconds value since the clock started ticking.
 *
 * @returns Number of milliseconds since the clock started.
 * @param   pTm    The timekeeping manager to use.
 */
uint32_t TMGetMillies(PTM pTm);

/**
 * Delay execution for the given amount of milliseconds.
 *
 * @returns nothing.
 * @param   pTm      The timekeeping manager to use.
 * @param   cMillies Amount of milliseconds to delay.
 */
void TMDelayMillies(PTM pTm, uint32_t cMillies);

/**
 * Register a new timer callback with the timekeeping manager.
 *
 * @returns status code.
 * @retval  ERR_TM_OUT_OF_SLOTS if there is no empty slot left.
 * @param   pTm       The timekeeping manager to use.
 * @param   pfnTmClbk The callback to call.
 * @param   pvUser    Opaque user data to pass in the callback.
 * @param   ppTmSlot  Where to store the pointer to the slot on success.
 */
int TMCallbackRegister(PTM pTm, PFNTMCLBK pfnTmClbk, void *pvUser, PTMCLBKSLOT *ppTmSlot);

/**
 * Deregister the given timer slot.
 *
 * @returns status code.
 * @param   pTm      The timekeeping manager to use.
 * @param   pTmSlot  The slot to deregister.
 *
 * @note Never ever deregister a timer slot in a timer callback
 *       and make sure that TMTick() is not called during that operation.
 */
int TMCallbackDeregister(PTM pTm, PTMCLBKSLOT pTmSlot);

/**
 * Sets the expiration timer for the given slot to the absolute amount of
 * milliseconds since the clock started ticking.
 *
 * @returns status code.
 * @param   pTm      The timekeeping manager to use.
 * @param   pTmSlot  The timer slot to set the expiration for.
 * @param   cMillies The absolute amount in milliseconds when the timer should expire.
 */
int TMCallbackSetExpirationAbsolute(PTM pTm, PTMCLBKSLOT pTmSlot, uint32_t cMillies);

/**
 * Sets the expiration timer for the given slot to the relative amount of
 * milliseconds starting from the current clock value.
 *
 * @returns status code.
 * @param   pTm      The timekeeping manager to use.
 * @param   pTmSlot  The timer slot to set the expiration for.
 * @param   cMillies The relative amount in milliseconds when the timer should expire.
 */
int TMCallbackSetExpirationRelative(PTM pTm, PTMCLBKSLOT pTmSlot, uint32_t cMillies);

/**
 * Stops the timer on the given slot.
 *
 * @returns status code.
 * @param   pTm      The timekeeping manager to use.
 * @param   pTmSlot  The timer slot to stop.
 */
int TMCallbackStop(PTM pTm, PTMCLBKSLOT pTmSlot);

#endif /* __tm_h */
