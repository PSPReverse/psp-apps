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
    /* volatile uint32_t* flash = (uint32_t*)0x2000500; */
    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pIfIoDev;

    /* UART supports only 1 byte wide register accesses. */
    if (cbWrite != 1) return ERR_INVALID_STATE;

    /* *flash++ = (uint32_t)pThis->pvUart + offReg; */
    /* *flash++ = *(uint8_t*)pvBuf; */
    *(volatile uint8_t *)((uintptr_t)pThis->pvUart + offReg) = *(uint8_t *)pvBuf;
    /* *flash++ = *(volatile uint8_t *)((uintptr_t)pThis->pvUart + offReg); */
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
    (void)hPduTransp;
    /* Nothing to do. */
    return INF_SUCCESS;
}


static int pspStubUartTranspBegin(PSPPDUTRANSP hPduTransp)
{
    (void)hPduTransp;
    /* Nothing to do. */
    return INF_SUCCESS;
}


static void pspStubUartTranspTerm(PSPPDUTRANSP hPduTransp)
{
    (void)hPduTransp;
    /* Nothing to do. */
}

#define ACPIMMIO_AOAC_BASE		0xfed81e00
#define FCH_UART_LEGACY_DECODE		0xfedc0020
#define   FCH_LEGACY_3F8_SH		3
static void test() {
    /* uint32_t* flash = (uint32_t*)0x2000000; */
    uint8_t* aoac = 0;

    uint16_t* uart_leg_decode = 0;

    uint8_t uart0_d3_ctl = 22 + 0x40;
    uint8_t uart0_d3_state = 22 + 0x40 + 1;

    uint32_t idx = 0 ;
    uint16_t uart_leg = (idx << 8) | (idx << 10) | (idx << 12) | (idx << 14);
		uart_leg |= 1 << FCH_LEGACY_3F8_SH;

    int rc = pspSerialStubX86PhysMap(FCH_UART_LEGACY_DECODE, true, (void**)&uart_leg_decode);
    if (!rc)
    {
        /* *flash = *uart_leg_decode; */
        *uart_leg_decode = uart_leg;
        /* *(flash+1) = *uart_leg_decode; */

        pspSerialStubX86PhysUnmapByPtr(uart_leg_decode);
    }


    rc = pspSerialStubX86PhysMap(ACPIMMIO_AOAC_BASE, true, (void**)&aoac);
    if (!rc)
    {
        /* *(flash + 50) = 0xD3; */
        uint8_t val = *(aoac + uart0_d3_state);
        /* *(flash + 51) = val; */

        val = *(aoac + uart0_d3_ctl);
        /* *(flash + 52) = val; */
        val |= (1 << 3);
        *(aoac + uart0_d3_ctl) = val;
    }
    pspSerialStubDelayMs(100);

    uint8_t val = *(aoac + uart0_d3_ctl);
    /* *(flash + 53) = val; */
    val = *(aoac + uart0_d3_state);
    /* *(flash + 54) = val; */
    pspSerialStubX86PhysUnmapByPtr(aoac);

}

#define AMD_GPIO_MUX_MASK			0x03
#define GPIO_138_IOMUX_UART0_TXD 0
#define GPIO_136_IOMUX_UART0_RXD 0
#define ACPIMMIO_IOMUX_BASE		0xfed80d00
#define ACPIMMIO_GPIO2_BASE		0xfed81700
#define AMD_GPIO_CONTROL_MASK			0x00f4ff00
#define GPIO_BANK2_CONTROL(gpio) (ACPIMMIO_GPIO2_BASE + (((gpio) - 128) * 4))
#define GPIO_PULL_PULL_NONE 0
static void configure_uart_gpio(void)
{
    uint32_t uart0_tx_func = 0;
    uint32_t uart0_rx_func = 0;
    uint32_t uart0_gpio_tx = 138;
    uint32_t uart0_gpio_rx = 136;
    /* volatile uint32_t* flash = (uint32_t*)0x2000000; */
    volatile uint8_t* addr;
    X86PADDR gpio_addr;
    uint32_t* gpio_reg;

    int rc = pspSerialStubX86PhysMap(ACPIMMIO_IOMUX_BASE + uart0_gpio_rx, true, (void**)&addr);
    if (!rc)
    {
		    /* iomux_write8(gpio, mux & AMD_GPIO_MUX_MASK); */
        *addr = uart0_rx_func & AMD_GPIO_MUX_MASK;
        /* *(flash + 42) = *addr; */

		    /* iomux_read8(gpio); /1* Flush posted write *1/ */
        (void volatile)*addr;
        pspSerialStubX86PhysUnmapByPtr((void*)addr);

    } else
        /* *(flash + 42) = 0xDEADBEEF; */

    gpio_addr = (X86PADDR)GPIO_BANK2_CONTROL(uart0_gpio_rx);

    rc = pspSerialStubX86PhysMap(gpio_addr,true, (void**)&gpio_reg);
    if (!rc)
    {

        volatile uint32_t reg;
        reg = *gpio_reg;
        reg &= ~AMD_GPIO_CONTROL_MASK;
        reg |= GPIO_PULL_PULL_NONE;
        *gpio_reg = reg;
        /* *(flash + 43) = *gpio_reg; */
        pspSerialStubX86PhysUnmapByPtr((void*)gpio_reg);
    }
        /* *(flash + 43) = 0xDEADBEEF; */
}

static uint32_t read_gpio_bank2(uint32_t gpio)
{
    X86PADDR pvGpioBankReg = GPIO_BANK2_CONTROL(gpio);
    volatile uint32_t* pvBankReg;
    uint32_t pvVal = 0xDEADBEEF;
    
    int rc = pspSerialStubX86PhysMap(pvGpioBankReg, true, (void**)&pvBankReg);
    if (!rc)
    {
        pvVal = *pvBankReg;
        pspSerialStubX86PhysUnmapByPtr((void*)pvBankReg);
    }

    return pvVal;
}

static void write_gpio_bank2(uint32_t gpio, uint32_t val)
{
    X86PADDR pvGpioBankReg = GPIO_BANK2_CONTROL(gpio);
    volatile uint32_t* pvBankReg;
    
    int rc = pspSerialStubX86PhysMap(pvGpioBankReg, true, (void**)&pvBankReg);
    if (!rc)
    {
        *pvBankReg = val;
        pspSerialStubX86PhysUnmapByPtr((void*)pvBankReg);
    }
}

#define ACPIMMIO_IOMUX_BASE		0xfed80d00
static uint8_t read_gpio_iomux(uint32_t gpio) {

    X86PADDR pvGpioIomux = (ACPIMMIO_IOMUX_BASE + gpio);
    volatile uint8_t* pvGpio;
    uint8_t pvVal = 0;

    int rc = pspSerialStubX86PhysMap(pvGpioIomux, true, (void**)&pvGpio);
    if (!rc)
    {
        pvVal = *pvGpio;
        pspSerialStubX86PhysUnmapByPtr((void*)pvGpio);
    }

    return pvVal;
}

static void write_gpio_iomux(uint32_t gpio, uint8_t val) {

    X86PADDR pvGpioIomux = (ACPIMMIO_IOMUX_BASE + gpio);
    volatile uint8_t* pvGpio;

    int rc = pspSerialStubX86PhysMap(pvGpioIomux, true, (void**)&pvGpio);
    if (!rc)
    {
        *pvGpio = val;
        pspSerialStubX86PhysUnmapByPtr((void*)pvGpio);
    }

}


void read_gpio_state() {
    // UART 0
    uint32_t gpio_138 = 138; // TX
    uint32_t gpio_136 = 136; // RX
    // UART 1
    uint32_t gpio_143 = 143; // TX
    uint32_t gpio_141 = 141; // RX
    // UART 2
    uint32_t gpio_137 = 137; // TX
    uint32_t gpio_135 = 135; // RX
    //UART 3
    uint32_t gpio_140 = 140; // TX
    uint32_t gpio_142 = 142; // RX

    uint32_t val = 0;


    /* volatile uint32_t* flash = (uint32_t*)0x2000100; */
    /* *flash++ = 0xDEADBEEF; */

    /* *flash++ = read_gpio_iomux(gpio_138); */
    /* *flash++ = read_gpio_iomux(gpio_136); */
    /* *flash++ = read_gpio_iomux(gpio_143); */
    /* *flash++ = read_gpio_iomux(gpio_141); */
    /* *flash++ = read_gpio_iomux(gpio_137); */
    /* *flash++ = read_gpio_iomux(gpio_135); */
    /* *flash++ = read_gpio_iomux(gpio_140); */
    /* *flash++ = read_gpio_iomux(gpio_142); */

    write_gpio_iomux(gpio_138,0); // UART 0 TX
    write_gpio_iomux(gpio_136,0); // UART 0 RX
    write_gpio_iomux(gpio_143,0); // UART 1 TX
    write_gpio_iomux(gpio_141,0); // UART 1 RX
    write_gpio_iomux(gpio_137,1); // UART 2 RX
    write_gpio_iomux(gpio_135,1); // UART 2 TX
    write_gpio_iomux(gpio_140,1); // UART 3 TX
    write_gpio_iomux(gpio_142,1); // UART 3 RX

    /* *flash++ = read_gpio_iomux(gpio_138); */
    /* *flash++ = read_gpio_iomux(gpio_136); */
    /* *flash++ = read_gpio_iomux(gpio_143); */
    /* *flash++ = read_gpio_iomux(gpio_141); */
    /* *flash++ = read_gpio_iomux(gpio_137); */
    /* *flash++ = read_gpio_iomux(gpio_135); */
    /* *flash++ = read_gpio_iomux(gpio_140); */
    /* *flash++ = read_gpio_iomux(gpio_142); */

    /* val = read_gpio_bank2(gpio_138); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* /1* write_gpio_bank2(gpio_138, val); *1/ */
    /* write_gpio_bank2(gpio_138, 0); */
    /* val = read_gpio_bank2(gpio_138); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_136); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* /1* write_gpio_bank2(gpio_136, val); *1/ */
    /* write_gpio_bank2(gpio_136, 0); */
    /* val = read_gpio_bank2(gpio_136); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_143); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_143, val); */
    /* val = read_gpio_bank2(gpio_143); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_141); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_141, val); */
    /* val = read_gpio_bank2(gpio_141); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_137); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_137, val); */
    /* val = read_gpio_bank2(gpio_137); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_135); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_135, val); */
    /* val = read_gpio_bank2(gpio_135); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_140); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_140, val); */
    /* val = read_gpio_bank2(gpio_140); */ 
    /* *flash++ = val; */

    /* val = read_gpio_bank2(gpio_142); */ 
    /* *flash++ = val; */
    /* val &= ~AMD_GPIO_CONTROL_MASK; */
    /* write_gpio_bank2(gpio_142, val); */
    /* val = read_gpio_bank2(gpio_142); */ 
    /* *flash++ = val; */

}

static void dump_pw()
{
    /* volatile uint32_t *flash = (uint32_t*)0x2000000; */
    uint32_t* pvAddr;
    /* *flash++ = 0x1; */
    int rc = pspSerialStubX86PhysMap(0xfed80300, true, (void**)&pvAddr);
    if (!rc)
    {
        /* *flash++ = *pvAddr; */
        /* *flash++ = *(pvAddr + 1); */

        *((uint8_t*)pvAddr + 59) = (uint8_t)0x1;
        /* *flash++ = *((uint8_t*)pvAddr + 59); */
        
    }

    /* *flash++ = 0x2; */

}

static void test_uart()
{
    /* volatile uint32_t *flash = (uint32_t*)0x2000200; */
    /* X86PADDR uart0 = 0xfedc9000; */
    /* X86PADDR uart1 = 0xfedca000; */
    /* X86PADDR uart2 = 0xfedce000; */
    /* X86PADDR uart3 = 0xfedcf000; */
    /* X86PADDR uartXX = 0xfedc3000; */
    X86PADDR uartYY = 0xfffdfc0003f8;

    /* X86PADDR uarts[5] = { uart0, uart1, uart2, uart3, uartXX}; */
    X86PADDR uarts[1] = { uartYY};

    int i;

    for (i = 0; i < 1; i++) {
        volatile uint8_t* pvUart;
        int rc = pspSerialStubX86PhysMap(uarts[i], true, (void**)&pvUart);
        if (!rc)
        {
            /* *flash++ = *pvUart; */
            /* *flash++ = *(pvUart + 1); */
            /* *flash++ = *(pvUart + 2); */
            /* *flash++ = *(pvUart + 3); */
            /* *flash++ = *(pvUart + 4); */
            /* *flash++ = *(pvUart + 5); */
            /* *flash++ = *(pvUart + 6); */
            /* *flash++ = *(pvUart + 7); */

            /* *pvUart = 0x41414141; */
        
            /* *flash++ = *pvUart; */
            /* *flash++ = *(pvUart + 1); */
            /* *flash++ = *(pvUart + 2); */
            /* *flash++ = *(pvUart + 3); */
            /* *flash++ = *(pvUart + 4); */
            /* *flash++ = *(pvUart + 5); */
            /* *flash++ = *(pvUart + 6); */
            /* *flash++ = *(pvUart + 7); */

            /* *flash++ = 0x1; */
            pspSerialStubX86PhysUnmapByPtr((void*)pvUart);
            /* *flash++ = 0x2; */


        }
        
        /* *flash++ = 0x3; */

    }

}

static void enable_espi() {

    /* volatile uint32_t* flash = (uint32_t*)0x2001100; */
    X86PADDR pvEspiDevD3Ctl = 0xfed81e76;
    volatile uint8_t* pvAddr;

    /* *flash++ = 0x1; */

    int rc =pspSerialStubX86PhysMap(pvEspiDevD3Ctl, true, (void**)&pvAddr);
    if (!rc)
    {
        /* *flash++ = 0x2; */
        uint8_t ctl = *pvAddr;
        uint8_t state = *(pvAddr + 1);
        /* *flash++ = 0x3; */
        /* *flash++ = ctl; */
        /* *flash++ = state; */

        ctl |= (1 << 3);

        *pvAddr = ctl;
        /* *flash++ = *pvAddr; */

        /* *flash++ = *(pvAddr + 1); */
        /* *flash++ = *(pvAddr + 1); */
        /* *flash++ = *(pvAddr + 1); */

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);

    }

}

/* The D14F3 PCI space seems to be pressent at 0xfffe000a3000.
 * See PPR_17h_18h_revB1.pdf:
 * D14F3x044 [IO Port Decode Enable] (FCH::ITF::LPC::IOPortDecodeEn)
 * Enable decode for Serial0 (port 0x3f8) and Serial1 (port 0x2f8)
 */
static void enable_io_uart_decode() {
    X86PADDR pvIoDecodeEn = 0xfffe000a3044;
    volatile uint32_t* pvAddr;
    /* volatile uint32_t* flash = (uint32_t*)0x2001000; */

    /* *flash++ = 0x1; */

    int rc = pspSerialStubX86PhysMap(pvIoDecodeEn, true, (void**)&pvAddr);
    if (!rc)
    {
        /* *flash++ = 0x2; */
        uint32_t val = *pvAddr;
        /* *flash++ = val; */
        val |= 0xc0;
        *pvAddr = val;
        val = *pvAddr;
        /* *flash++ = val; */

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);


    }
    /* *flash++ = 0x3; */

}

static void set_pci_d14_f3_rsvd_func() 
{

    X86PADDR pvPciDev14F3 = 0xfffe000a30d0;
    volatile uint32_t* pvAddr;
    /* volatile uint32_t* flash = (uint32_t*)0x2001200; */

    /* *flash++ = 1; */

    int rc = pspSerialStubX86PhysMap(pvPciDev14F3, true, (void**)&pvAddr);
    if (!rc)
    {
        /* *flash++ = 2; */
        uint32_t val = *pvAddr;
        /* *flash++ = val; */
        val = val & 0xfff9ffff | 0x40000;
        *pvAddr = val;
        /* *flash++ = *pvAddr; */

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
    /* *flash++ = 3; */


}

static void set_unknown_espi_reg()
{
    X86PADDR pvEspiUnknown = 0xfec20040;
    volatile uint32_t* pvAddr;
    /* volatile uint32_t* flash = (uint32_t*)0x2001300; */

    /* *flash++ = 1; */

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
        /* *flash++ = 2; */
        uint32_t val = *pvAddr;
        /* *flash++ = val; */
        val &= 0xfffffffb;
        *pvAddr = val;
        /* *flash++ = *pvAddr; */

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
    /* *flash++ = 3; */

}

static void enable_superio_port()
{
    X86PADDR pvEspiUnknown = 0xfffe000a3048;
    volatile uint32_t* pvAddr;

    LogRel("enable_superio_port:\n");

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
        uint32_t val = *pvAddr;
        LogRel("0xfffe000a3048: %#x\n", val);
        val |= 0x7 | BIT(5);
        *pvAddr = val;
        LogRel("0xfffe000a3048: %#x\n", *pvAddr);

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
}

static void map_superio_1640()
{
    X86PADDR pvEspiUnknown = 0xfffe000a3064;
    volatile uint32_t* pvAddr;

    LogRel("map_superio_1640:\n");

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
        uint32_t val = *pvAddr;
        LogRel("0xfffe000a3064: %#x\n", val);
        /* val = 0x1640; */
        val = 0; /*val = 0x1600;*/
        *pvAddr = val;
        LogRel("0xfffe000a3064: %#x\n", *pvAddr);

        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
}

static uint8_t aspeed_reg_read(volatile uint8_t *pbBase, uint8_t uReg)
{
    *pbBase = uReg;
    return *(pbBase + 1);
}

static void unknown_smn()
{
    volatile uint32_t *pvAddr;
    LogRel("Enable unknown smn:\n");
    
    int rc =pspSerialStubSmnMap(0x2dc58d0,(void**)&pvAddr);
    if (!rc)
    {
        uint32_t val = *pvAddr;
        LogRel("Pre SMN 0x2dc58d0: %#x\n", val);
        val = val & 0xfff9ffff | 0x40000;
        *pvAddr = val;
        LogRel("Post SMN 0x2dc58d0: %#x\n", *pvAddr);
        pspSerialStubSmnUnmapByPtr( (void*)pvAddr);
    }

}

static void configure_superio()
{
    X86PADDR pvEspiUnknown = 0xfffdfc00002e;
    volatile uint8_t* pvAddr;

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
#if 0
        *pvAddr = 0x55;
        *pvAddr = 0x2;
        *(pvAddr + 1) = *(pvAddr+1) & ~0x0 | 0x80;
        *pvAddr = 0xc;
        *(pvAddr + 1) = *(pvAddr+1) & ~0x38 | 0x80;
        *pvAddr = 0x25;
        *(pvAddr + 1) = *(pvAddr+1) & ~0x1u | 0xfe;
        *pvAddr = 0x28;
        *(pvAddr + 1) = *(pvAddr+1) & ~0xf | 0x4;
        *pvAddr = 0xaa;
#else
        *pvAddr = 0xa5;
        *pvAddr = 0xa5;
        for (unsigned i = 0x21; i < 0x30; i++)
            LogRel("Register %#x: %#x\n", i, aspeed_reg_read(pvAddr, i));
        *pvAddr = 0x7;
        *(pvAddr + 1) = 0x2;
        *pvAddr = 0x61;
        *(pvAddr + 1) = 0xf8;
        *pvAddr = 0x60;
        *(pvAddr + 1) = 0x3;
        *pvAddr = 0x30;
        *(pvAddr + 1) = 0x1;
        *pvAddr = 0xaa;      
#endif
        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
}

static void dump_lpc_gating(void)
{
    X86PADDR pvEspiUnknown = 0xfed803ec;
    volatile uint8_t* pvAddr;

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
        LogRel("dump_lpc_gating: %#x\n", *pvAddr);
        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
}

static void dump_lpc_aoac(void)
{
    X86PADDR pvEspiUnknown = 0xfed81e00 + 0x48;
    volatile uint8_t* pvAddr;

    int rc = pspSerialStubX86PhysMap(pvEspiUnknown, true, (void**)&pvAddr);
    if (!rc)
    {
        LogRel("dump_lpc_aoac: %#x %#x\n", *pvAddr, *(pvAddr + 1));
        pspSerialStubX86PhysUnmapByPtr((void*)pvAddr);
    }
}

static int pspStubUartTranspInit(void *pvMem, size_t cbMem, PPSPPDUTRANSP phPduTransp)
{
    (void)phPduTransp;

    if (cbMem < sizeof(PSPPDUTRANSPINT))
        return ERR_INVALID_PARAMETER;

    PPSPPDUTRANSPINT pThis = (PPSPPDUTRANSPINT)pvMem;

    pThis->PhysX86UartBase     = 0xfffdfc0003f8;
    /* pThis->PhysX86UartBase     = 0xfedc9000; */
    /* pThis->PhysX86UartBase     = 0xfedca000; */
    /* pThis->PhysX86UartBase     = 0xfedc3000; */
    /* pThis->PhysX86UartBase     = 0xfedce000; */
    /* pThis->PhysX86UartBase     = 0xfedcf000; */
    pThis->pvUart              = NULL;
    pThis->IfIoDev.pfnRegRead  = pspStubX86UartRegRead;
    pThis->IfIoDev.pfnRegWrite = pspStubX86UartRegWrite;

#if 0
    set_pci_d14_f3_rsvd_func();
    enable_espi();
    set_unknown_espi_reg();
#endif
    dump_lpc_aoac();
    dump_lpc_gating();
    enable_io_uart_decode();
    map_superio_1640();
    enable_superio_port();
    unknown_smn();
    configure_superio();
    /* dump_pw(); */
    /* read_gpio_state(); */
    /* configure_uart_gpio(); */
    /* test(); */

    /* volatile uint32_t* flash = (uint32_t*)0x2000400; */
    /* uint32_t val = 0; */
    /* volatile uint8_t* addr = 0; */
    /* int rc = pspSerialStubX86PhysMap(0xfedca000, true, (void**)&addr); */
    /* if (!rc) */ 
    /* { */
    /*     *(addr) = 'A'; */
    /*     pspSerialStubX86PhysUnmapByPtr((void*)addr); */
    /* } */

    /* *(flash) = 0x1; */

    int rc = pspSerialStubX86PhysMap(pThis->PhysX86UartBase, true /*fMmio*/, (void **)&pThis->pvUart);
    if (!rc)
    {
        /* *(flash) = 0x2; */
        rc = PSPUartCreate(&pThis->Uart, &pThis->IfIoDev);
        if (!rc)
        {
            /* *(flash) = 0x3; */
            rc = PSPUartParamsSet(&pThis->Uart, 115200, PSPUARTDATABITS_8BITS, PSPUARTPARITY_NONE, PSPUARTSTOPBITS_1BIT);
            if (!rc) {
                /* *(flash) = 0x4; */
                /* test_uart(); */
                /* *(flash) = 0x5; */
                uint32_t val = 0x41;
                size_t written = 0;
                PSPUartWrite(&pThis->Uart, &val, 1, &written);
                /* *(flash) = 0x6; */
                /* *(flash + 15) = written; */
            }
        }
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

