/** @file
 * PSP internal interfaces - PDU transport channel callback table.
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
#ifndef __include_pdu_transp_h
#define __include_pdu_transp_h

#if defined(IN_PSP)
# include <common/types.h>
#else
# error "Invalid environment"
#endif

/** PDU transport channel instance handle. */
typedef struct PSPPDUTRANSPINT *PSPPDUTRANSP;
/** Pointer to a PDU transport channel instance handle. */
typedef PSPPDUTRANSP *PPSPPDUTRANSP;


/** Pointer to a PDU transport channel interface. */
typedef struct PSPPDUTRANSPIF *PPSPPDUTRANSPIF;
/** Pointer to a const PDU transport channel interface. */
typedef const struct PSPPDUTRANSPIF *PCPSPPDUTRANSPIF;


/** PDU transport channel interface. */
typedef struct PSPPDUTRANSPIF
{
    /** Number if bytes of private state required. */
    size_t              cbState;

    /**
     * Initialize the transport channel.
     *
     * @returns Status code.
     * @param   pvMem               The memory available for the PDU transport channel instance.
     * @param   cbMem               Number of bytes available.
     * @param   phPduTransp         Where to store the handle on success.
     */
    int         (*pfnInit)  (void *pvMem, size_t cbMem, PPSPPDUTRANSP phPduTransp);

    /**
     * Terminate the transport channel.
     *
     * @returns nothing.
     * @param   hPduTransp          PDU transport channel instance handle.
     */
    void        (*pfnTerm) (PSPPDUTRANSP hPduTransp);

    /**
     * Marks the begin of a channel access.
     *
     * @returns Status code.
     * @param   hPduTransp          PDU transport channel instance handle.
     */
    int         (*pfnBegin) (PSPPDUTRANSP hPduTransp);

    /**
     * Marks the end of a channel access.
     *
     * @returns Status code.
     * @param   hPduTransp          PDU transport channel instance handle.
     */
    int         (*pfnEnd) (PSPPDUTRANSP hPduTransp);

    /**
     * Return the amount of bytes available for reading.
     *
     * @returns Number of bytes available for reading.
     * @param   hPduTransp          PDU transport channel instance handle.
     */
    size_t      (*pfnPeek) (PSPPDUTRANSP hPduTransp);

    /**
     * Data read callback.
     *
     * @returns Status code.
     * @param   hPduTransp          PDU transport channel instance handle.
     * @param   pvBuf               Where to store the read data.
     * @param   cbRead              How much to read.
     * @param   pcbRead             Where to store the number of bytes read upon success, optional.
     */
    int         (*pfnRead) (PSPPDUTRANSP hPduTransp, void *pvBuf, size_t cbRead, size_t *pcbRead);

    /**
     * Data write callback.
     *
     * @returns Status code.
     * @param   hPduTransp          PDU transport channel instance handle.
     * @param   pvBuf               The data to write.
     * @param   cbWrite             How much to write.
     * @param   pcbWritten          Where to store the number of bytes written upon success, optional.
     */
    int         (*pfnWrite) (PSPPDUTRANSP hPduTransp, const void *pvBuf, size_t cbWrite, size_t *pcbWritten);

} PSPPDUTRANSPIF;


#endif /* !__include_pdu_transp_h */

