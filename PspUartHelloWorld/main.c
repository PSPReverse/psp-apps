/** @file PSP Hello World. 
*/

/*
 * Copyright (C) 2020 Alexander Eichner <alexander.eichner@campus.tu-berlin.de>
 * Copyright (C) 2020 Robert Buhren <robert.buhren@sect.tu-berlin.de>
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
#include <uart.h>
#include <x86-map.h>
#include <smn-map.h>
#include <types.h>
#include <io.h>
#include <string.h>
#include <common/status.h>
#include <err.h>
#include <mmio.h>
#include <misc.h>
#include <platform.h>

/**
 * x86 UART device I/O interface.
 */
typedef struct PSPPDUTRANSPINT
{
    /** Device I/O interface. */
    PSPIODEVIF                  IfIoDev;
    /** The MMIO mapping of the UART. */
    volatile void               *pvUart;
} PSPPDUTRANSPINT;

/** Pointer to the x86 UART PDU transport channel instance. */
typedef PSPPDUTRANSPINT *PPSPPDUTRANSPINT;

/**
 * x86 UART register read callback.
 */
static int pspUartRegRead(PCPSPIODEVIF pIfIoDev, uint32_t offReg, void *pvBuf, size_t cbRead)
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
static int pspUartRegWrite(PCPSPIODEVIF pIfIoDev, uint32_t offReg, const void *pvBuf, size_t cbWrite)
{
    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pIfIoDev;

    /* UART supports only 1 byte wide register accesses. */
    if (cbWrite != 1) return ERR_INVALID_STATE;

    *(volatile uint8_t *)((uintptr_t)pThis->pvUart + offReg) = *(uint8_t *)pvBuf;
    return INF_SUCCESS;
}

void ExcpUndefInsn(PPSPIRQREGFRAME pRegFrame) {
  (void)pRegFrame;
  /* For now, just stall the execution */
  do {} while(1);
}

void ExcpSwi(void) {
  /* For now, just stall the execution */
  do {} while(1);
}

void ExcpPrefAbrt(PPSPIRQREGFRAME pRegFrame) {
  (void)pRegFrame;
  /* For now, just stall the execution */
  do {} while(1);
}

void ExcpDataAbrt(PPSPIRQREGFRAME pRegFrame) {
  (void)pRegFrame;
  /* For now, just stall the execution */
  do {} while(1);
}

void ExcpIrq(PPSPIRQREGFRAME pRegFrame) {
  (void)pRegFrame;
  /* For now, just stall the execution */
  do {} while(1);
}

void ExcpFiq(void) {
  /* For now, just stall the execution */
  do {} while(1);
}

int main(void)
{
    PSPUART uart;
    PSPPDUTRANSPINT ptrans;
    ptrans.IfIoDev.pfnRegWrite = pspUartRegWrite;
    ptrans.IfIoDev.pfnRegRead = pspUartRegRead;
    ptrans.pvUart = NULL;

   /* Disable IRQs */ 
    pspIrqDisable();

    /* Don't do anything if this is not the master PSP. */
    /* @todo Figure out why this is required to get the UART working. */
    if (pspGetPhysDieId() != 0)
    {
        for (;;);
    }

    /* Basic initialization of the SOC and the SuperIO. */
    pspPlatformInit();

    /* Map the memory-mapped UART ioports into the PSP address space */
    pspX86PhysMap(0xfffdfc0003f8, true, (void**)&ptrans.pvUart);

    /* Create the UART object and initialize the UART */
    PSPUartCreate(&uart, &(ptrans.IfIoDev));
    PSPUartParamsSet(&uart, 115200, PSPUARTDATABITS_8BITS, PSPUARTPARITY_NONE, PSPUARTSTOPBITS_1BIT);

    /* Write "Hello World!" to the uart */
    PSPUartWrite(&uart, "Hello World!\n", sizeof("Hello World!\n"), NULL);

    for(;;);
}
