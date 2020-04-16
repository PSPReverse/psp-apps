/** @file
 * PSP app - SPI PDU transport channel when the Dediprog EM100 is used.
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


#define PSP_SPI_MASTER_SMN_ADDR         0x02dc4000
#define PSP_SPI_MASTER_SPEED_CFG        0x22
#define PSP_SPI_MASTER_CMD_CODE         0x45
#define PSP_SPI_MASTER_CMD_TRIG         0x47
# define PSP_SPI_MASTER_CMD_TRIG_BIT    BIT(7)
#define PSP_SPI_MASTER_TX_CNT           0x48
#define PSP_SPI_MASTER_RX_CNT           0x4b
#define PSP_SPI_MASTER_STATUS           0x4c
# define PSP_SPI_MASTER_STATUS_BSY      BIT(31)
#define PSP_SPI_FIFO_START              0x80


#define PSP_SPI_MASTER_CHUNK_SZ         64

/**
 * SPI flash transport channel.
 */
typedef struct PSPPDUTRANSPINT
{
    /** SMN mapping for the SPI master. */
    volatile void               *pvSmnMap;
    /** Amount of data available for reading. */
    size_t                      cbAvail;
    /** The read chunk. */
    uint8_t                     abChunk[PSP_SPI_MASTER_CHUNK_SZ];
    /** Offset into the chunk buffer. */
    uint8_t                     offChunk;
} PSPPDUTRANSPINT;
/** Pointer to the x86 UART PDU transport channel instance. */
typedef PSPPDUTRANSPINT *PPSPPDUTRANSPINT;


static inline void pspStubSpiMasterWriteRegU8(PPSPPDUTRANSPINT pThis, uint32_t offReg, uint8_t bVal)
{
    *((volatile uint8_t *)pThis->pvSmnMap + offReg) = bVal;
}


static inline uint8_t pspStubSpiMasterReadRegU8(PPSPPDUTRANSPINT pThis, uint32_t offReg)
{
    return *((volatile uint8_t *)pThis->pvSmnMap + offReg);
}


static inline void pspStubSpiMasterWriteRegU16(PPSPPDUTRANSPINT pThis, uint32_t offReg, uint16_t u16Val)
{
    *(volatile uint16_t *)((volatile uint8_t *)pThis->pvSmnMap + offReg) = u16Val;
}


static inline void pspStubSpiMasterWriteRegU32(PPSPPDUTRANSPINT pThis, uint32_t offReg, uint32_t u32Val)
{
    *(volatile uint32_t *)((volatile uint8_t *)pThis->pvSmnMap + offReg) = u32Val;
}


static inline uint32_t pspStubSpiMasterReadRegU32(PPSPPDUTRANSPINT pThis, uint32_t offReg)
{
    return *(volatile uint32_t *)((volatile uint8_t *)pThis->pvSmnMap + offReg);
}


/**
 * Executes a single SPI transaction on the SPI master.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   bCmd                The command byte.
 * @param   pbTx                The data to transfer.
 * @param   cbTx                Number of bytes to transmit.
 * @param   pbRx                Where to store the received bytes.
 * @param   cbRx                Number of bytes to receive.
 */
static int pspStubSpiMasterXact(PPSPPDUTRANSPINT pThis, uint8_t bCmd, uint8_t *pbTx, size_t cbTx,
                                uint8_t *pbRx, size_t cbRx)
{
    /* Wait until the master is idling. */
    while (pspStubSpiMasterReadRegU32(pThis, PSP_SPI_MASTER_STATUS) & PSP_SPI_MASTER_STATUS_BSY);

    pspStubSpiMasterWriteRegU8(pThis, PSP_SPI_MASTER_CMD_CODE, bCmd);
    pspStubSpiMasterWriteRegU8(pThis, PSP_SPI_MASTER_TX_CNT,   (uint8_t)cbTx);
    pspStubSpiMasterWriteRegU8(pThis, PSP_SPI_MASTER_RX_CNT,   (uint8_t)cbRx);

    for (uint32_t i = 0; i < cbTx; i++)
        pspStubSpiMasterWriteRegU8(pThis, PSP_SPI_FIFO_START + i, pbTx[i]);

    pspStubSpiMasterWriteRegU8(pThis, PSP_SPI_MASTER_CMD_TRIG, PSP_SPI_MASTER_CMD_TRIG_BIT); /* Issues the transaction */

    /* Wait until the master is idling. */
    while (pspStubSpiMasterReadRegU32(pThis, PSP_SPI_MASTER_STATUS) & PSP_SPI_MASTER_STATUS_BSY);

    for (uint32_t i = 0; i < cbRx; i++)
        pbRx[i + cbTx] = pspStubSpiMasterReadRegU8(pThis, PSP_SPI_FIFO_START + cbTx + i);

    return INF_SUCCESS;
}


/**
 * Reads a given register from the EM100.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   idxReg              Register index.
 * @param   bReg                The value to write.
 */
static int pspStubEm100RegWrite(PPSPPDUTRANSPINT pThis, uint8_t idxReg, uint8_t bReg)
{
    uint8_t abCmd[3] = { 0 };
    abCmd[1] = 0xa0 | (idxReg & 0xf);
    abCmd[2] = bReg;

    return pspStubSpiMasterXact(pThis, 0x11, &abCmd[0], sizeof(abCmd),
                                NULL /*pbRx*/, 0 /*cbRx*/);
}


/**
 * Reads a given register from the EM100.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   idxReg              Register index.
 * @param   pbReg               Where to store the read value on success.
 */
static int pspStubEm100RegRead(PPSPPDUTRANSPINT pThis, uint8_t idxReg, uint8_t *pbReg)
{
    uint8_t abCmd[2] = { 0 };
    uint8_t abRecv[4] = { 0 };
    abCmd[1] = 0xb0 | (idxReg & 0xf);

    int rc = pspStubSpiMasterXact(pThis, 0x11, &abCmd[0], sizeof(abCmd),
                                  &abRecv[0], sizeof(abRecv));
    if (!rc)
        *pbReg = abRecv[3];

    return rc;
}


/**
 * Writes to the upload FIFO of the em100.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   pbBuf               The data to write.
 * @param   cbWrite             Number of bytes to write.
 */
static int pspStubEm100UFifoWrite(PPSPPDUTRANSPINT pThis, const uint8_t *pbBuf, size_t cbWrite)
{
    uint8_t abData[PSP_SPI_MASTER_CHUNK_SZ + 2 + 2];

    if (cbWrite > PSP_SPI_MASTER_CHUNK_SZ)
        return ERR_INVALID_PARAMETER;

    abData[0] = 0x0;
    abData[1] = 0xc0;
    abData[2] = 0xef;
    abData[3] = (uint8_t)cbWrite;
    for (uint32_t i = 0; i < cbWrite; i++)
        abData[i + 4] = pbBuf[i];

    return pspStubSpiMasterXact(pThis, 0x11, &abData[0], cbWrite + 4,
                                NULL /*pbRx*/, 0 /*cbRx*/);
}


/**
 * Reads from the download FIFO of the em100.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   pbBuf               Where to store the data read.
 * @param   cbRead              Number of bytes to read.
 */
static int pspStubEm100DFifoRead(PPSPPDUTRANSPINT pThis, uint8_t *pbBuf, size_t cbRead)
{
    if (cbRead > PSP_SPI_MASTER_CHUNK_SZ)
        return ERR_INVALID_PARAMETER;

    uint8_t abCmd[3];
    uint8_t abRecv[PSP_SPI_MASTER_CHUNK_SZ + sizeof(abCmd)];
    abCmd[0] = 0x0;
    abCmd[1] = 0xd0;

    int rc = pspStubSpiMasterXact(pThis, 0x11, &abCmd[0], sizeof(abCmd),
                                  &abRecv[0], cbRead + sizeof(abCmd));
    if (!rc)
    {
        for (uint32_t i = 0; i < cbRead; i++)
            pbBuf[i] = abRecv[i + sizeof(abCmd)];
    }

    /* Notify the other end that we cleared the FIFO. */
    abCmd[0] = 0x0;
    abCmd[1] = 0xc0;
    abCmd[2] = 0xdf;
    return pspStubSpiMasterXact(pThis, 0x11, &abCmd[0], sizeof(abCmd),
                                NULL /*pbRx*/, 0 /*cbRx*/);
}


/**
 * Queries the number of bytes used in the uFIFO.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   pcbUsed             Where to store the number of bytes used on success.
 */
static int pspStubEm100UFifoQueryUsed(PPSPPDUTRANSPINT pThis, size_t *pcbUsed)
{
    *pcbUsed = 0;

    /* Check number of bytes used by reading the master and uFIFO length register. */
    uint8_t bMainReg = 0;
    int rc = pspStubEm100RegRead(pThis, 0, &bMainReg);
    if (   !rc
        && (bMainReg & BIT(5)) == 0) /* At least one valid byte in upload FIFO? */
    {
        uint8_t bUFifoLength = 0;
        rc = pspStubEm100RegRead(pThis, 1, &bUFifoLength);
        if (!rc)
        {
            bMainReg >>= 1; /* Get to the 8th bit of the uFIFO length. */
            *pcbUsed = (((uint16_t)bMainReg & 1) << 8) | bUFifoLength;
        }
    }

    return rc;
}


/**
 * Queries the number of bytes available in the dFIFO.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 * @param   pcbAvail            Where to store the number of bytes available on success.
 */
static int pspStubEm100DFifoQueryAvail(PPSPPDUTRANSPINT pThis, size_t *pcbAvail)
{
    *pcbAvail = 0;

    /* Check number of bytes available by reading the master and dFIFO length register. */
    uint8_t bMainReg = 0;
    int rc = pspStubEm100RegRead(pThis, 0, &bMainReg);
    if (   !rc
        && (bMainReg & BIT(6)) == 0) /* At least one valid byte in download FIFO? */
    {
        uint8_t bDFifoLength = 0;
        int rc = pspStubEm100RegRead(pThis, 2, &bDFifoLength);
        if (!rc)
        {
            bMainReg >>= 3; /* Get to the 8th bit of the dFIFO length. */
            size_t cbAvail = (((uint16_t)bMainReg & 1) << 8) | bDFifoLength;
            *pcbAvail = cbAvail;
        }
    }

    return rc;
}


/**
 * Fetch the next chunk of data from the dFIFO if available.
 *
 * @returns Status code.
 * @param   pThis               The EM100 transport channel instance.
 *
 * @note This is required here as the EM100 clears the dFIFO after each read no matter
 *       how many bytes were transfered. So we have to cache chunks in case the caller
 *       wants to read less data.
 */
static int pspStubEm100FetchChunk(PPSPPDUTRANSPINT pThis)
{
    if (pThis->cbAvail)
        return INF_SUCCESS;

    /*
     * Wait a bit for the number of bytes available to stabilize
     * or we risk that we get an old number of bytes available and
     * read less than what is actually inside the dFIFO which is cleared
     * afterwards.
     */
    size_t cbAvail = 0;
    int rc = INF_SUCCESS;
    for (;;)
    {
        pspSerialStubDelayMs(1);

        size_t cbThisAvail = 0;
        rc = pspStubEm100DFifoQueryAvail(pThis, &cbThisAvail);
        if (   rc
            || cbThisAvail == cbAvail
            || cbThisAvail == PSP_SPI_MASTER_CHUNK_SZ)
        {
            cbAvail = cbThisAvail;
            break;
        }

        cbAvail = cbThisAvail;
    }

    /* We should never have more than chunk size here. */
    if (   !rc
        && cbAvail)
    {
        rc = pspStubEm100DFifoRead(pThis, &pThis->abChunk[0], cbAvail);
        if (!rc)
        {
            pThis->cbAvail  = cbAvail;
            pThis->offChunk = 0;
        }
    }

    return rc;
}


static int pspStubEm100TranspWrite(PSPPDUTRANSP hPduTransp, const void *pvBuf, size_t cbWrite, size_t *pcbWritten)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    uint8_t *pbBuf = (uint8_t *)pvBuf;
    int rc = INF_SUCCESS;
    while (   cbWrite
           && rc == INF_SUCCESS)
    {
        size_t cbThisWrite = MIN(cbWrite, PSP_SPI_MASTER_CHUNK_SZ);
        rc = pspStubEm100UFifoWrite(pThis, pbBuf, cbThisWrite);
        if (!rc)
        {
            pbBuf   += cbThisWrite;
            cbWrite -= cbThisWrite;
        }
    }

    return INF_SUCCESS;
}


static int pspStubEm100TranspRead(PSPPDUTRANSP hPduTransp, void *pvBuf, size_t cbRead, size_t *pcbRead)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    uint8_t *pbBuf = (uint8_t *)pvBuf;
    size_t cbReadLeft = cbRead;
    int rc = INF_SUCCESS;
    while (   cbReadLeft
           && rc == INF_SUCCESS)
    {
        /* fetch a new chunk if we're out of data. */
        while (   !pThis->cbAvail
               && rc == INF_SUCCESS)
            rc = pspStubEm100FetchChunk(pThis);

        size_t cbThisRead = MIN(cbReadLeft, pThis->cbAvail);
        memcpy(pbBuf, &pThis->abChunk[pThis->offChunk], cbThisRead);
        pbBuf           += cbThisRead;
        cbReadLeft      -= cbThisRead;
        pThis->cbAvail  -= cbThisRead;
        pThis->offChunk += cbThisRead;
    }

    return rc;
}


static size_t pspStubEm100TranspPeek(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    if (pThis->cbAvail)
        return pThis->cbAvail;

    size_t cbAvail = 0;
    pspStubEm100DFifoQueryAvail(pThis, &cbAvail);
    return cbAvail;
}


static int pspStubEm100TranspEnd(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
    return INF_SUCCESS;
}


static int pspStubEm100TranspBegin(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
    return INF_SUCCESS;
}


static void pspStubEm100TranspTerm(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;
    pspSerialStubSmnUnmapByPtr((void *)pThis->pvSmnMap);
}


static int pspStubEm100TranspInit(void *pvMem, size_t cbMem, PPSPPDUTRANSP phPduTransp)
{
    if (cbMem < sizeof(PSPPDUTRANSPINT))
        return ERR_INVALID_PARAMETER;

    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pvMem;
    int rc = pspSerialStubSmnMap(PSP_SPI_MASTER_SMN_ADDR, (void **)&pThis->pvSmnMap);
    if (!rc)
    {
#if 0 /** @todo Doesn't has any effect. */
        pspStubSpiMasterWriteRegU16(pThis, PSP_SPI_MASTER_SPEED_CFG, 0x5555); /* Switches everything to 800kHz mode. */
#endif

        /* Check for the EM100 identifier. */
        uint8_t bId = 0;
        rc = pspStubEm100RegRead(pThis, 3, &bId);
        if (!rc)
        {
            if (bId == 0xaa)
            {
                pThis->cbAvail  = 0;
                pThis->offChunk = 0;
                *phPduTransp = pThis;
                return INF_SUCCESS;
            }
            else
                rc = -1;
        }

        pspSerialStubSmnUnmapByPtr((void *)pThis->pvSmnMap);
    }

    return rc;
}


const PSPPDUTRANSPIF g_SpiFlashTranspEm100 =
{
    /** cbState */
    sizeof(PSPPDUTRANSPINT),
    /** pfnInit */
    pspStubEm100TranspInit,
    /** pfnTerm */
    pspStubEm100TranspTerm,
    /** pfnBegin */
    pspStubEm100TranspBegin,
    /** pfnEnd */
    pspStubEm100TranspEnd,
    /** pfnPeek */
    pspStubEm100TranspPeek,
    /** pfnRead */
    pspStubEm100TranspRead,
    /** pfnWrite */
    pspStubEm100TranspWrite
};

