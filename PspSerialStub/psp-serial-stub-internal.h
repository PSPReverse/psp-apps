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
#ifndef __include_psp_serial_stub_internal_h
#define __include_psp_serial_stub_internal_h

#if defined(IN_PSP)
# include <common/types.h>
#else
# error "Invalid environment"
#endif

/**
 * Maps the given x86 physical address into the PSP address space.
 *
 * @returns Status code.
 * @param   PhysX86Addr             The x86 physical address to map.
 * @param   fMmio                   Flag whether this a MMIO address.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspSerialStubX86PhysMap(X86PADDR PhysX86Addr, bool fMmio, void **ppv);


/**
 * Unmaps a previously mapped x86 physical address.
 *
 * @returns Status code.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubX86PhysMap().
 */
int pspSerialStubX86PhysUnmapByPtr(void *pv);


/**
 * Unmaps a previously mapped SMN address.
 *
 * @returns Status code.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubSmnMap().
 */
int pspSerialStubSmnMap(SMNADDR SmnAddr, void **ppv);


/**
 * Maps the given SMN address into the PSP address space.
 *
 * @returns Status code.
 * @param   SmnAddr                 The SMN address to map.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
int pspSerialStubSmnUnmapByPtr(void *pv);


/**
 * Wait the given number of milli seconds.
 *
 * @returns nothing.
 * @param   cMillies                Number of milli seconds to wait.
 */
void pspSerialStubDelayMs(uint32_t cMillies);


/**
 * Wait the given number of microseconds.
 *
 * @returns nothing.
 * @param   cMicros                Number of microseconds to wait.
 */
void pspSerialStubDelayUs(uint64_t cMicros);

#endif /* !__include_psp_serial_stub_internal_h */

