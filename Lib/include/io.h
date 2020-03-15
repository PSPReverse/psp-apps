/** @file
 * PSP internal interfaces - Device register read/write callbacks.
 */

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __include_io_h
#define __include_io_h

#if defined(IN_PSP)
# include <common/types.h>
#else
# error "Invalid environment"
#endif

/** Pointer to a I/O device interface. */
typedef struct PSPIODEVIF *PPSPIODEVIF;
/** Pointer to a const I/O device interface. */
typedef const struct PSPIODEVIF *PCPSPIODEVIF;


/** I/O device interface. */
typedef struct PSPIODEVIF
{
    /**
     * Register read callback.
     *
     * @returns Status code.
     * @param   pIfIoDev            Pointer to the interface containing the callback.
     * @param   offReg              Offset of the register to read from.
     * @param   pvBuf               Where to store the read data.
     * @param   cbRead              How much to read.
     */
    int         (*pfnRegRead) (PCPSPIODEVIF pIfIoDev, uint32_t offReg, void *pvBuf, size_t cbRead);

    /**
     * Register write callback.
     *
     * @returns Status code.
     * @param   pIfIoDev            Pointer to the interface containing the callback.
     * @param   offReg              Offset of the register to start writing to.
     * @param   pvBuf               The data to write.
     * @param   cbRead              How much to write.
     */
    int         (*pfnRegWrite) (PCPSPIODEVIF pIfIoDev, uint32_t offReg, const void *pvBuf, size_t cbWrite);

} PSPIODEVIF;


/**
 * Reads from the register a the given offset.
 *
 * @returns
 */
static inline int PSPIoDevRegRead(PCPSPIODEVIF pIfIoDev, uint32_t offReg, void *pvBuf, size_t cbRead)
{
    return pIfIoDev->pfnRegRead(pIfIoDev, offReg, pvBuf, cbRead);
}


static inline int PSPIoDevRegWrite(PCPSPIODEVIF pIfIoDev, uint32_t offReg, const void *pvBuf, size_t cbWrite)
{
    return pIfIoDev->pfnRegWrite(pIfIoDev, offReg, pvBuf, cbWrite);
}

#endif /* !__include_uart_h */

