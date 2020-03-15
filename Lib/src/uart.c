/** @file
 * Basic x86 UART device driver.
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

#include <x86/uart.h>

#include <err.h>
#include <uart.h>


/**
 * Sets the UART divisor.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   uDivisor                The divisor to set.
 */
static int pspUartDivisorSet(PPSPUART pUart, uint32_t uDivisor)
{
    uint8_t uDivLatchL = uDivisor & 0xff;
    uint8_t uDivLatchH = (uDivisor >> 8) & 0xff;

    /* Read LCR and set DLAB. */
    uint8_t uLcr = 0;
    int rc = PSPIoDevRegRead(pUart->pIfDevIo, X86_UART_REG_LCR_OFF, &uLcr, sizeof(uLcr));
    if (rc == INF_SUCCESS)
    {
        uLcr |= X86_UART_REG_LCR_DLAB;
        rc = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_LCR_OFF, &uLcr, sizeof(uLcr));
        if (rc == INF_SUCCESS)
        {
            rc = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_DL_LSB_OFF, &uDivLatchL, sizeof(uDivLatchL));
            if (rc == INF_SUCCESS)
                rc = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_DL_MSB_OFF, &uDivLatchH, sizeof(uDivLatchH));

            /* Clear DLAB again. */
            uLcr &= ~X86_UART_REG_LCR_DLAB;
            int rc2 = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_LCR_OFF, &uLcr, sizeof(uLcr));
            if (rc == INF_SUCCESS)
                rc = rc2;
        }
    }

    return rc;
}


int PSPUartCreate(PPSPUART pUart, PCPSPIODEVIF pIfDevIo)
{
    pUart->pIfDevIo = pIfDevIo;

    /* Bring the device into a known state. */

    /* Disable all interrupts. */
    uint8_t uTmp = 0;
    int rc = PSPIoDevRegWrite(pIfDevIo, X86_UART_REG_IER_OFF, &uTmp, sizeof(uTmp));
    if (rc == INF_SUCCESS)
    {
        /* Disable FIFO. */
        rc = PSPIoDevRegWrite(pIfDevIo, X86_UART_REG_FCR_OFF, &uTmp, sizeof(uTmp));
        if (rc == INF_SUCCESS)
        {
            /* Set known line parameters. */
            rc = PSPUartParamsSet(pUart, 115200, PSPUARTDATABITS_8BITS,
                                  PSPUARTPARITY_NONE, PSPUARTSTOPBITS_1BIT);
        }
    }

    return rc;
}


void PSPUartDestroy(PPSPUART pUart)
{
    pUart->pIfDevIo = NULL;
}


int PSPUartParamsSet(PPSPUART pUart, uint32_t uBps, PSPUARTDATABITS enmDataBits,
                     PSPUARTPARITY enmParity, PSPUARTSTOPBITS enmStopBits)
{
    uint32_t uDivisor = 115200 / uBps; /* For PC compatible UARTs using a 1.8432 MHz crystal. */
    uint8_t uLcr = 0;

    switch (enmDataBits)
    {
        case PSPUARTDATABITS_8BITS:
            X86_UART_REG_LCR_WLS_SET(uLcr, X86_UART_REG_LCR_WLS_8);
            break;
        default:
            return ERR_INVALID_PARAMETER;
    }

    switch (enmParity)
    {
        case PSPUARTPARITY_NONE:
            uLcr &= ~X86_UART_REG_LCR_PEN;
            break;
        default:
            return ERR_INVALID_PARAMETER;
    }

    switch (enmStopBits)
    {
        case PSPUARTSTOPBITS_1BIT:
            uLcr &= ~X86_UART_REG_LCR_STB;
            break;
        default:
            return ERR_INVALID_PARAMETER;
    }

    int rc = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_LCR_OFF, &uLcr, sizeof(uLcr));
    if (!rc)
        rc = pspUartDivisorSet(pUart, uDivisor);

    return rc;
}


size_t PSPUartGetDataAvail(PPSPUART pUart)
{
    size_t cbAvail = 0;
    uint8_t uLsr = 0;
    int rc = PSPIoDevRegRead(pUart->pIfDevIo, X86_UART_REG_LSR_OFF, &uLsr, sizeof(uLsr));
    if (   rc == INF_SUCCESS
        && uLsr & X86_UART_REG_LSR_DR)
        cbAvail++;

    return cbAvail;
}


size_t PSPUartGetTxSpaceAvail(PPSPUART pUart)
{
    size_t cbAvail = 0;
    uint8_t uLsr = 0;
    int rc = PSPIoDevRegRead(pUart->pIfDevIo, X86_UART_REG_LSR_OFF, &uLsr, sizeof(uLsr));
    if (   rc == INF_SUCCESS
        && uLsr & X86_UART_REG_LSR_THRE)
        cbAvail++;

    return cbAvail;
}


int PSPUartRead(PPSPUART pUart, void *pvBuf, size_t cbRead, size_t *pcbRead)
{
    size_t cbActuallyRead = 0;
    uint8_t *pbBuf = (uint8_t *)pvBuf;
    int rc = INF_SUCCESS;

    do
    {
        while (PSPUartGetDataAvail(pUart) == 0); /* Wait until there is something to read. */

        size_t cbThisRead = 0;
        rc = PSPUartReadNB(pUart, pbBuf, cbRead, &cbThisRead);
        if (rc != INF_SUCCESS)
            break;

        pbBuf          += cbThisRead;
        cbRead         -= cbThisRead;
        cbActuallyRead += cbThisRead;
    } while (   cbRead
             && rc == INF_SUCCESS);

    if (   rc == INF_SUCCESS
        && pcbRead)
        *pcbRead = cbActuallyRead;

    return rc;
}


int PSPUartReadNB(PPSPUART pUart, void *pvBuf, size_t cbRead, size_t *pcbRead)
{
    int rc = INF_SUCCESS;

    *pcbRead = 0;

    if (PSPUartGetDataAvail(pUart) > 0)
    {
        rc = PSPIoDevRegRead(pUart->pIfDevIo, X86_UART_REG_RBR_OFF, pvBuf, 1);
        *pcbRead = 1;
    }
    else
        rc = INF_TRY_AGAIN;

    return rc;
}


int PSPUartWrite(PPSPUART pUart, const void *pvBuf, size_t cbWrite, size_t *pcbWritten)
{
    size_t cbWritten = 0;
    const uint8_t *pbBuf = (const uint8_t *)pvBuf;
    int rc = INF_SUCCESS;

    do
    {
        while (PSPUartGetTxSpaceAvail(pUart) == 0); /* Wait until there is room to write. */

        size_t cbThisWritten = 0;
        rc = PSPUartWriteNB(pUart, pbBuf, cbWrite, &cbThisWritten);
        if (rc != INF_SUCCESS)
            break;

        pbBuf     += cbThisWritten;
        cbWrite   -= cbThisWritten;
        cbWritten += cbThisWritten;
    } while (   cbWrite
             && rc == INF_SUCCESS);

    if (   rc == INF_SUCCESS
        && pcbWritten)
        *pcbWritten = cbWritten;

    return rc;
}


int PSPUartWriteNB(PPSPUART pUart, const void *pvBuf, size_t cbWrite, size_t *pcbWritten)
{
    int rc = INF_SUCCESS;

    *pcbWritten = 0;

    if (PSPUartGetTxSpaceAvail(pUart) > 0)
    {
        rc = PSPIoDevRegWrite(pUart->pIfDevIo, X86_UART_REG_THR_OFF, pvBuf, 1);
        *pcbWritten = 1;
    }
    else
        rc = INF_TRY_AGAIN;

    return rc;
}

