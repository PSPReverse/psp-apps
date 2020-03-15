/** @file
 * PSP internal interfaces - UART driver.
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
#ifndef __include_uart_h
#define __include_uart_h

#if defined(IN_PSP)
# include <common/types.h>
#else
# error "Invalid environment"
#endif

#include <io.h>


/**
 * UART data bits config enum.
 */
typedef enum PSPUARTDATABITS
{
    /** Invalid value, do not use. */
    PSPUARTDATABITS_INVALID = 0,
    /** 8 data bits. */
    PSPUARTDATABITS_8BITS,
    /** 32bit hack. */
    PSPUARTDATABITS_32BIT_HACK = 0x7fffffff
} PSPUARTDATABITS;


/**
 * UART parity config enum.
 */
typedef enum PSPUARTPARITY
{
    /** Invalid value, do not use. */
    PSPUARTPARITY_INVALID = 0,
    /** No parity. */
    PSPUARTPARITY_NONE,
    /** 32bit hack. */
    PSPUARTPARITY_32BIT_HACK = 0x7fffffff
} PSPUARTPARITY;


/**
 * UART stop bits config enum.
 */
typedef enum PSPUARTSTOPBITS
{
    /** Invalid value, do not use. */
    PSPUARTSTOPBITS_INVALID = 0,
    /** 1 stop bit. */
    PSPUARTSTOPBITS_1BIT,
    /** 32bit hack. */
    PSPUARTSTOPBITS_32BIT_HACK = 0x7fffffff
} PSPUARTSTOPBITS;


/**
 * UART driver instance state, treat as private.
 */
typedef struct PSPUART
{
    /** Pointer to the device I/O interface given during construction. */
    PCPSPIODEVIF        pIfDevIo;
    /** @todo State members. */
} PSPUART;
/** Pointer to a UART driver instance. */
typedef PSPUART *PPSPUART;
/** Pointer to a const UART driver instance. */
typedef const PSPUART *PCPSPUART;


/**
 * Create a new UART driver instance.
 *
 * @returns Status code.
 * @param   pUart                   Pointer to the uninitialized UART driver instance.
 * @param   pIfDevIo                The device I/O interface pointer.
 */
int PSPUartCreate(PPSPUART pUart, PCPSPIODEVIF pIfDevIo);


/**
 * Destroys the given UART driver instance.
 *
 * @returns nothing.
 * @param   pUart                   The UART driver instance to destroy.
 */
void PSPUartDestroy(PPSPUART pUart);


/**
 * Sets the UART connection parameters.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   uBps                    Baud rate to configure.
 * @param   enmDataBits             Number of data bits to use.
 * @param   enmParity               The parity to use.
 * @param   enmStopBits             Number of stop bits to use.
 */
int PSPUartParamsSet(PPSPUART pUart, uint32_t uBps, PSPUARTDATABITS enmDataBits,
                     PSPUARTPARITY enmParity, PSPUARTSTOPBITS enmStopBits);


/**
 * Returns the number of bytes available for reading.
 *
 * @returns Number of bytes available for reading, 0 if nothing is available.
 * @param   pUart                   The UART driver instance.
 */
size_t PSPUartGetDataAvail(PPSPUART pUart);


/**
 * Returns the amount of bytes available in the transmitter queue.
 *
 * @returns Number of bytes available in the transmitter queue.
 * @param   pUart                   The UART driver instance.
 */
size_t PSPUartGetTxSpaceAvail(PPSPUART pUart);


/**
 * Read the given amount of data from the given UART device instance.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  Number of bytes to read.
 * @param   pcbRead                 Where to store the number of bytes actually read, optional.
 */
int PSPUartRead(PPSPUART pUart, void *pvBuf, size_t cbRead, size_t *pcbRead);


/**
 * Read the given amount of data from the given UART device instance - non blocking.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  Number of bytes to read.
 * @param   pcbRead                 Where to store the number of bytes actually read.
 */
int PSPUartReadNB(PPSPUART pUart, void *pvBuf, size_t cbRead, size_t *pcbRead);


/**
 * Write the given amount of data to the given UART device instance.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   pvBuf                   The data to write.
 * @param   cbWrite                 Number of bytes to write.
 * @param   pcbWritten              Where to store the number of bytes actually written, optional.
 */
int PSPUartWrite(PPSPUART pUart, const void *pvBuf, size_t cbWrite, size_t *pcbWritten);


/**
 * Write the given amount of data to the given UART device instance - non blocking.
 *
 * @returns Status code.
 * @param   pUart                   The UART driver instance.
 * @param   pvBuf                   The data to write.
 * @param   cbWrite                 Number of bytes to write.
 * @param   pcbWritten              Where to store the number of bytes actually written.
 */
int PSPUartWriteNB(PPSPUART pUart, const void *pvBuf, size_t cbWrite, size_t *pcbWritten);


#endif /* !__include_uart_h */

