/** @file
 * PSP app - SPI flash PDU transport channel.
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
#include <string.h>
#include <err.h>
#include <log.h>

#include <io.h>
#include <uart.h>

#include "pdu-transp.h"
#include "psp-serial-stub-internal.h"


#define PSP_SPI_FLASH_SMN_ADDR          0x0a000000

#define PSP_SPI_FLASH_LOCK_WAIT         5

/** Where in the flash the message channel is located. */
#define SPI_MSG_CHAN_HDR_OFF            0xaab000
#define SPI_MSG_CHAN_AVAIL_OFF          0xaaa000
#define SPI_MSG_CHAN_AVAIL_F_OFF        0xaac000
#define SPI_FLASH_LOCK_OFF              0xaa0000
#define SPI_FLASH_LOCK_UNLOCK_REQ_MAGIC 0x19570528 /* (Frank Schaetzing) */
#define SPI_FLASH_LOCK_UNLOCKED_MAGIC   0x18280208 /* (Jules Verne) */
#define SPI_FLASH_LOCK_LOCK_REQ_MAGIC   0x19380110 /* (Donald E Knuth) */
#define SPI_FLASH_LOCK_LOCKED_MAGIC     0x18990223 /* (Erich Kaestner) */

#define SPI_MSG_CHAN_AVAIL_MAGIC        0x19640522 /* (Dan Brown) */

/**
 * SPI flash transport channel.
 */
typedef struct PSPPDUTRANSPINT
{
    /** Number of times the SPI flash was locked. */
    uint32_t                    cSpiFlashLock;
    /** Last accessed offset for reading (top optimize the cache wiping). */
    uint32_t                    offReadLast;
    /** Number of bytes available for reading. */
    size_t                      cbReadAvail;
} PSPPDUTRANSPINT;
/** Pointer to the x86 UART PDU transport channel instance. */
typedef PSPPDUTRANSPINT *PPSPPDUTRANSPINT;


static size_t pspStubSpiFlashTranspPeek(PSPPDUTRANSP hPduTransp);


/**
 * Wipe the read cache of the SPI flash.
 *
 * @returns nothing.
 * @param   pThis                   SPI flash transport instance data.
 */
static void pspStubSpiFlashWipeCache(PPSPPDUTRANSPINT pThis)
{
    /* Map the SMN region. */
    void *pvMap;
    int rc = pspSerialStubSmnMap(PSP_SPI_FLASH_SMN_ADDR, &pvMap);
    if (!rc)
    {
        /* Make sure we don't read cached data by issuing a read to a non accecssed region. */
        uint32_t uIgnored = *(volatile uint32_t *)pvMap;
        pspSerialStubSmnUnmapByPtr(pvMap);
    }
}


/**
 * Reads the given amount of data from the SPI flash.
 *
 * @returns Nothing.
 * @param   pThis                   SPI flash transport instance data.
 * @param   off                     Start offset to read from.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  How many bytes to read.
 */
static void pspStubSpiFlashRead(PPSPPDUTRANSPINT pThis, uint32_t off, void *pvBuf, size_t cbRead)
{
    if (   pThis->offReadLast == 0xffff0000
        || (   off >= pThis->offReadLast
            && off < pThis->offReadLast + 256 /* cache size */))
        pspStubSpiFlashWipeCache(pThis);

    /* Map the SMN region. */
    void *pvMap;
    int rc = pspSerialStubSmnMap(PSP_SPI_FLASH_SMN_ADDR + off, &pvMap);
    if (!rc)
    {
        memcpy(pvBuf, pvMap, cbRead);
        pspSerialStubSmnUnmapByPtr(pvMap);
        pThis->offReadLast = off;
    }
}


/**
 * Writes the given amount of data to the SPI flash.
 *
 * @returns Nothing.
 * @param   pThis                   SPI flash transport instance data.
 * @param   off                     Start offset to write to.
 * @param   pvBuf                   The data to write.
 * @param   cbWrite                 How many bytes to write.
 */
static void pspStubSpiFlashWrite(PPSPPDUTRANSPINT pThis, uint32_t off, const void *pvBuf, size_t cbWrite)
{
    /* Map the SMN region. */
    void *pvMap;
    int rc = pspSerialStubSmnMap(PSP_SPI_FLASH_SMN_ADDR + off, &pvMap);
    if (!rc)
    {
        volatile uint8_t *pbDst = (volatile uint8_t *)pvMap;
        const uint8_t *pbSrc = (const uint8_t *)pvBuf;

        while (cbWrite >= sizeof(uint32_t))
        {
            *(volatile uint32_t *)pbDst = *(uint32_t *)pbSrc;
            pbDst   += sizeof(uint32_t);
            pbSrc   += sizeof(uint32_t);
            cbWrite -= sizeof(uint32_t);
        }

        if (cbWrite)
            memcpy((uint8_t *)pbDst, pbSrc, cbWrite);
        pspSerialStubSmnUnmapByPtr(pvMap);
    }
}


/**
 * Lock the SPI flash for access.
 *
 * @returns nothing.
 * @param   pThis                   SPI flash transport instance data.
 */
static void pspStubSpiFlashLock(PPSPPDUTRANSPINT pThis)
{
    if (!pThis->cSpiFlashLock)
    {
        /* Write the lock request until we see the magic value for the locked case. */
        uint32_t u32Read = 0;

        uint32_t u32Write = SPI_FLASH_LOCK_LOCK_REQ_MAGIC;
        pspStubSpiFlashWrite(pThis, SPI_FLASH_LOCK_OFF, &u32Write, sizeof(u32Write));
        pspStubSpiFlashRead(pThis, 0, &u32Write, sizeof(u32Write)); /* Dummy */

        do
        {
            pspSerialStubDelayMs(PSP_SPI_FLASH_LOCK_WAIT); /* Wait a moment for the emulator process the request. */
            pspStubSpiFlashRead(pThis, SPI_FLASH_LOCK_OFF, &u32Read, sizeof(u32Read));
        }
        while (u32Read != SPI_FLASH_LOCK_LOCKED_MAGIC);
    }

    pThis->cSpiFlashLock++;
}


/**
 * Unlock the SPI flash for access.
 *
 * @returns nothing.
 * @param   pThis                   SPI flash transport instance data.
 */
static void pspStubSpiFlashUnlock(PPSPPDUTRANSPINT pThis)
{
    pThis->cSpiFlashLock--;
    if (!pThis->cSpiFlashLock)
    {
        /* Write the unlock request and wait till we see the magic value for the unlocked case. */
        uint32_t u32Read = 0;

        uint32_t u32Write = SPI_FLASH_LOCK_UNLOCK_REQ_MAGIC;
        pspStubSpiFlashWrite(pThis, SPI_FLASH_LOCK_OFF, &u32Write, sizeof(u32Write));
        pspStubSpiFlashRead(pThis, 0, &u32Write, sizeof(u32Write)); /* Dummy */

        do
        {
            pspSerialStubDelayMs(PSP_SPI_FLASH_LOCK_WAIT); /* Wait a moment for the emulator process the request. */
            pspStubSpiFlashRead(pThis, SPI_FLASH_LOCK_OFF, &u32Read, sizeof(u32Read));
        }
        while (u32Read != SPI_FLASH_LOCK_UNLOCKED_MAGIC);
    }
}


static int pspStubSpiFlashTranspWrite(PSPPDUTRANSP hPduTransp, const void *pvBuf, size_t cbWrite, size_t *pcbWritten)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    pspStubSpiFlashLock(pThis);
    size_t cbWriteLeft = cbWrite;
    uint8_t *pbBuf = (uint8_t *)pvBuf;

    while (cbWriteLeft)
    {
        size_t cbThisWrite = MIN(256, cbWriteLeft);

        pspStubSpiFlashWrite(pThis, SPI_MSG_CHAN_HDR_OFF, pbBuf, cbThisWrite);
        pbBuf       += cbThisWrite;
        cbWriteLeft -= cbThisWrite;
    }
    pspStubSpiFlashUnlock(pThis);

    return INF_SUCCESS;
}


static int pspStubSpiFlashTranspRead(PSPPDUTRANSP hPduTransp, void *pvBuf, size_t cbRead, size_t *pcbRead)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    size_t cbReadLeft = cbRead;
    uint8_t *pbBuf = (uint8_t *)pvBuf;

    while (cbReadLeft)
    {
        pspStubSpiFlashLock(pThis);

        /* Simple case, read the number of bytes available and get them. */
        size_t cbThisRead = MIN(pspStubSpiFlashTranspPeek(pThis), cbReadLeft);
        if (cbThisRead)
        {
            pspStubSpiFlashRead(pThis, SPI_MSG_CHAN_HDR_OFF, pbBuf, cbThisRead);
            pbBuf       += cbThisRead;
            cbReadLeft  -= cbThisRead;
            pspStubSpiFlashWrite(pThis, SPI_MSG_CHAN_AVAIL_OFF, &cbThisRead, sizeof(cbThisRead));
            pspStubSpiFlashRead(pThis, 0, &cbThisRead, sizeof(cbThisRead)); /* Dummy */
        }

#if 0
        pThis->cbReadAvail -= cbThisRead;
        /* Update the amount of data we can read here under the lock if we've read everything. */
        if (!pThis->cbReadAvail)
            pThis->cbReadAvail = pspStubSpiFlashTranspPeek(pThis);
#endif
        pspStubSpiFlashUnlock(pThis);
    }

    return INF_SUCCESS;
}


static size_t pspStubSpiFlashTranspPeek(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

#if 0
    /* Don't bother with checking if there is still something left. */
    if (pThis->cbReadAvail)
        return pThis->cbReadAvail;
#endif

    uint32_t u32Avail = 0;
    uint32_t u32ReadAvailMagic = 0;
    pspStubSpiFlashRead(pThis, SPI_MSG_CHAN_AVAIL_F_OFF, &u32ReadAvailMagic, sizeof(u32ReadAvailMagic));
    if (u32ReadAvailMagic == SPI_MSG_CHAN_AVAIL_MAGIC)
    {
        /*
         * The magic indicates something is available for reading, now do the locking and get the actual
         * number of bytes available.
         */
        pspStubSpiFlashLock(pThis);
        pspStubSpiFlashRead(pThis, SPI_MSG_CHAN_AVAIL_OFF, &u32Avail, sizeof(u32Avail));
        pspStubSpiFlashUnlock(pThis);
        pThis->cbReadAvail = u32Avail;
    }

    return u32Avail;
}


static int pspStubSpiFlashTranspEnd(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;
    pspStubSpiFlashUnlock(pThis);
    return INF_SUCCESS;
}


static int pspStubSpiFlashTranspBegin(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;
    pspStubSpiFlashLock(pThis);
    return INF_SUCCESS;
}


static void pspStubSpiFlashTranspTerm(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
}


static int pspStubSpiFlashTranspInit(void *pvMem, size_t cbMem, PPSPPDUTRANSP phPduTransp)
{
    if (cbMem < sizeof(PSPPDUTRANSPINT))
        return ERR_INVALID_PARAMETER;

    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pvMem;

    pThis->cSpiFlashLock = 0;
    pThis->offReadLast   = 0xffff0000; /* Invalid, this will always wipe the cache. */
    pThis->cbReadAvail   = 0;

    uint32_t u32Magic;
    do
    {
        pspStubSpiFlashRead(pThis, 0, &u32Magic, sizeof(u32Magic)); /* Read some dummy first, to flush the SPI read cache. */
        pspStubSpiFlashRead(pThis, SPI_FLASH_LOCK_OFF, &u32Magic, sizeof(u32Magic));
    }
    while (u32Magic != SPI_FLASH_LOCK_UNLOCKED_MAGIC);

    *phPduTransp = pThis;
    return INF_SUCCESS;
}


const PSPPDUTRANSPIF g_SpiFlashTransp =
{
    /** cbState */
    sizeof(PSPPDUTRANSPINT),
    /** pfnInit */
    pspStubSpiFlashTranspInit,
    /** pfnTerm */
    pspStubSpiFlashTranspTerm,
    /** pfnBegin */
    pspStubSpiFlashTranspBegin,
    /** pfnEnd */
    pspStubSpiFlashTranspEnd,
    /** pfnPeek */
    pspStubSpiFlashTranspPeek,
    /** pfnRead */
    pspStubSpiFlashTranspRead,
    /** pfnWrite */
    pspStubSpiFlashTranspWrite
};

