/** @file
 * PSP app - UART PDU transport channel.
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
#include <err.h>
#include <log.h>

#include <io.h>
#include <uart.h>

#include "pdu-transp.h"
#include "psp-serial-stub-internal.h"


/**
 * x86 UART device I/O interface.
 */
typedef struct PSPPDUTRANSPINT
{
    /** Device I/O interface. */
    PSPIODEVIF                  IfIoDev;
    /** The physical x86 address where the UART is mapped. */
    X86PADDR                    PhysX86UartBase;
    /** The MMIO mapping of the UART. */
    volatile void               *pvUart;
    /** UART device instance. */
    PSPUART                     Uart;
} PSPPDUTRANSPINT;
/** Pointer to the x86 UART PDU transport channel instance. */
typedef PSPPDUTRANSPINT *PPSPPDUTRANSPINT;


/**
 * x86 UART register read callback.
 */
static int pspStubX86UartRegRead(PCPSPIODEVIF pIfIoDev, uint32_t offReg, void *pvBuf, size_t cbRead)
{
    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pIfIoDev;

    /* UART supports only 1 byte wide register accesses. */
    if (cbRead != 1) return ERR_INVALID_STATE;

    *(uint8_t *)pvBuf = *(volatile uint8_t *)((uintptr_t)pThis->pvUart + offReg);
    return INF_SUCCESS;
}


/**
 * x86 UART register write callback.
 */
static int pspStubX86UartRegWrite(PCPSPIODEVIF pIfIoDev, uint32_t offReg, const void *pvBuf, size_t cbWrite)
{
    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pIfIoDev;

    /* UART supports only 1 byte wide register accesses. */
    if (cbWrite != 1) return ERR_INVALID_STATE;

    *(volatile uint8_t *)((uintptr_t)pThis->pvUart + offReg) = *(uint8_t *)pvBuf;
    return INF_SUCCESS;
}


static int pspStubUartTranspWrite(PSPPDUTRANSP hPduTransp, const void *pvBuf, size_t cbWrite, size_t *pcbWritten)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    return PSPUartWrite(&pThis->Uart, pvBuf, cbWrite, pcbWritten);
}


static int pspStubUartTranspRead(PSPPDUTRANSP hPduTransp, void *pvBuf, size_t cbRead, size_t *pcbRead)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    return PSPUartRead(&pThis->Uart, pvBuf, cbRead, pcbRead);
}


static size_t pspStubUartTranspPeek(PSPPDUTRANSP hPduTransp)
{
    PPSPPDUTRANSPINT pThis = hPduTransp;

    return PSPUartGetDataAvail(&pThis->Uart);
}


static int pspStubUartTranspEnd(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
    return INF_SUCCESS;
}


static int pspStubUartTranspBegin(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
    return INF_SUCCESS;
}


static void pspStubUartTranspTerm(PSPPDUTRANSP hPduTransp)
{
    /* Nothing to do. */
}


static int pspStubUartTranspInit(void *pvMem, size_t cbMem, PPSPPDUTRANSP phPduTransp)
{
    if (cbMem < sizeof(PSPPDUTRANSPINT))
        return ERR_INVALID_PARAMETER;

    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pvMem;

    pThis->PhysX86UartBase     = 0xfffdfc0003f8;
    pThis->pvUart              = NULL;
    pThis->IfIoDev.pfnRegRead  = pspStubX86UartRegRead;
    pThis->IfIoDev.pfnRegWrite = pspStubX86UartRegWrite;

    int rc = pspSerialStubX86PhysMap(pThis->PhysX86UartBase, true /*fMmio*/, (void **)&pThis->pvUart);
    if (!rc)
    {
        rc = PSPUartCreate(&pThis->Uart, &pThis->IfIoDev);
        if (!rc)
            rc = PSPUartParamsSet(&pThis->Uart, 115200, PSPUARTDATABITS_8BITS, PSPUARTPARITY_NONE, PSPUARTSTOPBITS_1BIT);
    }

    return rc;
}


const PSPPDUTRANSPIF g_UartTransp =
{
    /** cbState */
    sizeof(PSPPDUTRANSPINT),
    /** pfnInit */
    pspStubUartTranspInit,
    /** pfnTerm */
    pspStubUartTranspTerm,
    /** pfnBegin */
    pspStubUartTranspBegin,
    /** pfnEnd */
    pspStubUartTranspEnd,
    /** pfnPeek */
    pspStubUartTranspPeek,
    /** pfnRead */
    pspStubUartTranspRead,
    /** pfnWrite */
    pspStubUartTranspWrite
};

