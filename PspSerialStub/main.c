/** @file
 * PSP app - Serial stub running in SVC mode.
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
#include <tm.h>

#include <io.h>
#include <uart.h>

#include <psp-stub/psp-serial-stub.h>

#include "pdu-transp.h"

/** Use the SPI message channel instead of the UART. */
#define PSP_SERIAL_STUB_SPI_MSG_CHAN    1

/** Indefinite wait. */
#define PSP_SERIAL_STUB_INDEFINITE_WAIT 0xffffffff


/**
 * x86 memory mapping slot.
 */
typedef struct PSPX86MAPPING
{
    /** The base X86 address being mapped (aligned to a 64MB boundary). */
    X86PADDR                PhysX86AddrBase;
    /** The memory type being used. */
    uint32_t                uMemType;
    /** Reference counter for this mapping, the mapping gets cleaned up if it reaches 0. */
    uint32_t                cRefs;
} PSPX86MAPPING;
/** Pointer to an x86 memory mapping slot. */
typedef PSPX86MAPPING *PPSPX86MAPPING;
/** Pointer to a const x86 memory mapping slot. */
typedef const PSPX86MAPPING *PCPSPX86MAPPING;


/**
 * SMN mapping slot.
 */
typedef struct PSPSMNMAPPING
{
    /** Base SMN address being mapped (aligned to a 1MB boundary). */
    SMNADDR                 SmnAddrBase;
    /* Reference counter for this mapping, the mapping gets cleand up if it reaches 0. */
    uint32_t                cRefs;
} PSPSMNMAPPING;
/** Pointer to a SMN mapping slot. */
typedef PSPSMNMAPPING *PPSPSMNMAPPING;
/** Pointer to a const SMN mapping slot. */
typedef const PSPSMNMAPPING *PCPSPSMNMAPPING;


/**
 * Timekeeping related data.
 */
typedef struct PSPTIMER
{
    /** Timekeeping manager. */
    TM                          Tm;
    /** Last seen counter value of the 100MHz timer (10ns granularity). */
    uint32_t                    cCnts;
    /** Sub millisecond ticks seen since the internal clock counter increment. */
    uint32_t                    cSubMsTicks;
} PSPTIMER;
/** Pointer to a timer. */
typedef PSPTIMER *PPSPTIMER;


/**
 * PDU receive states.
 */
typedef enum PSPSERIALPDURECVSTATE
{
    /** Invalid receive state. */
    PSPSERIALPDURECVSTATE_INVALID = 0,
    /** Currently receiveing the header. */
    PSPSERIALPDURECVSTATE_HDR,
    /** Currently receiveing the payload. */
    PSPSERIALPDURECVSTATE_PAYLOAD,
    /** Currently receiving the footer. */
    PSPSERIALPDURECVSTATE_FOOTER,
    /** 32bit hack. */
    PSPSERIALPDURECVSTATE_32BIT_HACK = 0x7fffffff
} PSPSERIALPDURECVSTATE;


/**
 * Global stub instance.
 */
typedef struct PSPSTUBSTATE
{
    /** The logger instance to use. */
    LOGGER                      Logger;
    /** Timekeeping related stuff. */
    PSPTIMER                    Timer;
    /** The timekeeping manager used. */
    PTM                         pTm;
    /** Flag whether the SPI message channel is used over the UART as the data transport. */
    bool                        fSpiMsgChan;
    /** Selected transport channel. */
    PCPSPPDUTRANSPIF            pIfTransp;
    /** Handle to the PDU transport channel. */
    PSPPDUTRANSP                hPduTransp;
    /** Private transport channel instance data. */
    uint8_t                     abTranspData[128];
    /** x86 mapping bookkeeping data. */
    PSPX86MAPPING               aX86MapSlots[15];
    /** SMN mapping bookkeeping data. */
    PSPSMNMAPPING               aSmnMapSlots[32];
    /** Number of CCDs detected. */
    uint32_t                    cCcds;
    /** Flag whether someone is connected. */
    bool                        fConnected;
    /** Flag whether we are in the early logging over SPI phase. */
    bool                        fEarlyLogOverSpi;
    /** Early SPI logging mapping. */
    void                        *pvEarlySpiLog;
    /** Flag whether logging is enabled at all currently. */
    bool                        fLogEnabled;
    /** Number of beacons sent. */
    uint32_t                    cBeaconsSent;
    /** Number of PDUs sent so far. */
    uint32_t                    cPdusSent;
    /** Next PDU counter value expected for a received PDU. */
    uint32_t                    cPduRecvNext;
    /** The PDU receive state. */
    PSPSERIALPDURECVSTATE       enmPduRecvState;
    /** Number of bytes to receive remaining in the current state. */
    size_t                      cbPduRecvLeft;
    /** Current offset into the PDU buffer. */
    uint32_t                    offPduRecv;
    /** The PDU receive buffer. */
    uint8_t                     abPdu[_4K];
    /** Scratch space. */
    uint8_t                     abScratch[16 * _1K]; /** @todo Check alignment on 8 byte boundary. */
} PSPSTUBSTATE;
/** Pointer to the binary loader state. */
typedef PSPSTUBSTATE *PPSPSTUBSTATE;

#define PSP_SERIAL_STUB_EARLY_SPI_LOG_OFF 0x0

/** The global stub state. */
static PSPSTUBSTATE g_StubState;
static uint32_t off = 0;


extern const PSPPDUTRANSPIF g_UartTransp;
extern const PSPPDUTRANSPIF g_SpiFlashTransp;

/**
 * Available transport channels.
 */
static PCPSPPDUTRANSPIF g_aPduTransp[] =
{
    &g_UartTransp,
    &g_SpiFlashTransp
};


/**
 * Maps the given x86 physical address into the PSP address space.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   PhysX86Addr             The x86 physical address to map.
 * @param   fMmio                   Flag whether this a MMIO address.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
static int pspStubX86PhysMap(PPSPSTUBSTATE pThis, X86PADDR PhysX86Addr, bool fMmio, void **ppv)
{
    int rc = INF_SUCCESS;
    uint32_t uMemType = fMmio ? 0x6 : 0x4;

    /* Split physical address into 64MB aligned base and offset. */
    X86PADDR PhysX86AddrBase = (PhysX86Addr & ~(_64M - 1));
    uint32_t offStart = PhysX86Addr - PhysX86AddrBase;

    PPSPX86MAPPING pMapping = NULL;
    uint32_t idxSlot = 0;
    for (uint32_t i = fMmio ? 8 : 0; i < ELEMENTS(pThis->aX86MapSlots); i++)
    {
        if (   (   pThis->aX86MapSlots[i].PhysX86AddrBase == NIL_X86PADDR
                && pThis->aX86MapSlots[i].cRefs == 0)
            || (   pThis->aX86MapSlots[i].PhysX86AddrBase == PhysX86AddrBase
                && pThis->aX86MapSlots[i].uMemType == uMemType))
        {
            pMapping = &pThis->aX86MapSlots[i];
            idxSlot = i;
            break;
        }
    }

    if (pMapping)
    {
        if (pMapping->PhysX86AddrBase == NIL_X86PADDR)
        {
            /* Set up the mapping. */
            pMapping->uMemType         = uMemType;
            pMapping->PhysX86AddrBase  = PhysX86AddrBase;

            /* Program base address. */
            PSPADDR PspAddrSlotBase = 0x03230000 + idxSlot * 4 * sizeof(uint32_t);
            *(volatile uint32_t *)PspAddrSlotBase        = ((PhysX86AddrBase >> 32) << 6) | ((PhysX86AddrBase >> 26) & 0x3f);
            *(volatile uint32_t *)(PspAddrSlotBase + 4)  = 0x12; /* Unknown but fixed value. */
            *(volatile uint32_t *)(PspAddrSlotBase + 8)  = uMemType;
            *(volatile uint32_t *)(PspAddrSlotBase + 12) = uMemType;
            *(volatile uint32_t *)(0x032303e0 + idxSlot * sizeof(uint32_t)) = 0xffffffff;
            *(volatile uint32_t *)(0x032304d8 + idxSlot * sizeof(uint32_t)) = 0xc0000000;
            *(volatile uint32_t *)0x32305ec = 0x3333;
        }

        asm volatile("dsb #0xf\nisb #0xf\n": : :"memory");
        pMapping->cRefs++;
        *ppv = (void *)(0x04000000 + idxSlot * _64M + offStart);
    }
    else
        rc = ERR_INVALID_STATE;

    return rc;
}


/**
 * Unmaps a previously mapped x86 physical address.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubX86PhysMap().
 */
static int pspStubX86PhysUnmapByPtr(PPSPSTUBSTATE pThis, void *pv)
{
    int rc = INF_SUCCESS;
    uintptr_t PspAddrMapBase = ((uintptr_t)pv) & ~(_64M - 1);
    PspAddrMapBase -= 0x04000000;

    uint32_t idxSlot = PspAddrMapBase / _64M;
    if (   idxSlot < ELEMENTS(pThis->aX86MapSlots)
        && PspAddrMapBase % _64M == 0)
    {
        PPSPX86MAPPING pMapping = &pThis->aX86MapSlots[idxSlot];

        asm volatile("dsb #0xf\nisb #0xf\n": : :"memory");
        if (pMapping->cRefs > 0)
        {
            pMapping->cRefs--;

            /* Clear out the mapping if there is no reference held. */
            if (!pMapping->cRefs)
            {
                pMapping->uMemType        = 0;
                pMapping->PhysX86AddrBase = NIL_X86PADDR;

                PSPADDR PspAddrSlotBase = 0x03230000 + idxSlot * 4 * sizeof(uint32_t);
                *(volatile uint32_t *)PspAddrSlotBase        = 0;
                *(volatile uint32_t *)(PspAddrSlotBase + 4)  = 0; /* Unknown but fixed value. */
                *(volatile uint32_t *)(PspAddrSlotBase + 8)  = 0;
                *(volatile uint32_t *)(PspAddrSlotBase + 12) = 0;
                *(volatile uint32_t *)(0x032303e0 + idxSlot * sizeof(uint32_t)) = 0xffffffff;
                *(volatile uint32_t *)(0x032304d8 + idxSlot * sizeof(uint32_t)) = 0;
            }
        }
        else
            rc = ERR_INVALID_PARAMETER;
    }
    else
        rc = ERR_INVALID_PARAMETER;

    return rc;
}


/**
 * Maps the given SMN address into the PSP address space.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   SmnAddr                 The SMN address to map.
 * @param   ppv                     Where to store the pointer to the mapping on success.
 */
static int pspStubSmnMap(PPSPSTUBSTATE pThis, SMNADDR SmnAddr, void **ppv)
{
    int rc = INF_SUCCESS;

    /* Split physical address into 1MB aligned base and offset. */
    SMNADDR  SmnAddrBase = (SmnAddr & ~(_1M - 1));
    uint32_t offStart = SmnAddr - SmnAddrBase;

    PPSPSMNMAPPING pMapping = NULL;
    uint32_t idxSlot = 0;
    for (uint32_t i = 0; i < ELEMENTS(pThis->aSmnMapSlots); i++)
    {
        if (   (   pThis->aSmnMapSlots[i].SmnAddrBase == 0
                && pThis->aSmnMapSlots[i].cRefs == 0)
            || pThis->aSmnMapSlots[i].SmnAddrBase == SmnAddrBase)
        {
            pMapping = &pThis->aSmnMapSlots[i];
            idxSlot = i;
            break;
        }
    }

    if (pMapping)
    {
        if (pMapping->SmnAddrBase == 0)
        {
            /* Set up the mapping. */
            pMapping->SmnAddrBase = SmnAddrBase;

            /* Program base address. */
            PSPADDR PspAddrSlotBase = 0x03220000 + (idxSlot / 2) * sizeof(uint32_t);
            uint32_t u32RegSmnMapCtrl = *(volatile uint32_t *)PspAddrSlotBase;
            if (idxSlot & 0x1)
                u32RegSmnMapCtrl |= ((SmnAddrBase >> 20) << 16);
            else
                u32RegSmnMapCtrl |= SmnAddrBase >> 20;
            *(volatile uint32_t *)PspAddrSlotBase = u32RegSmnMapCtrl;
        }

        pMapping->cRefs++;
        *ppv = (void *)(0x01000000 + idxSlot * _1M + offStart);
    }
    else
        rc = ERR_INVALID_STATE;

    //LogRel("pspStubSmnUnmapByPtr: SmnAddr=%#x -> rc=%d,pv=%p\n", SmnAddr, rc, *ppv);
    return rc;
}


/**
 * Unmaps a previously mapped SMN address.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pv                      Pointer to the mapping as returned by a successful call to pspStubSmnMap().
 */
static int pspStubSmnUnmapByPtr(PPSPSTUBSTATE pThis, void *pv)
{
    int rc = INF_SUCCESS;
    uintptr_t PspAddrMapBase = ((uintptr_t)pv) & ~(_1M - 1);
    PspAddrMapBase -= 0x01000000;

    uint32_t idxSlot = PspAddrMapBase / _1M;
    if (   idxSlot < ELEMENTS(pThis->aSmnMapSlots)
        && PspAddrMapBase % _1M == 0)
    {
        PPSPSMNMAPPING pMapping = &pThis->aSmnMapSlots[idxSlot];

        if (pMapping->cRefs > 0)
        {
            pMapping->cRefs--;

            /* Clear out the mapping if there is no reference held. */
            if (!pMapping->cRefs)
            {
                pMapping->SmnAddrBase = 0;

                PSPADDR PspAddrSlotBase = 0x03220000 + (idxSlot / 2) * sizeof(uint32_t);
                uint32_t u32RegSmnMapCtrl = *(volatile uint32_t *)PspAddrSlotBase;
                if (idxSlot & 0x1)
                    u32RegSmnMapCtrl &= 0xffff;
                else
                    u32RegSmnMapCtrl &= 0xffff0000;
                *(volatile uint32_t *)PspAddrSlotBase = u32RegSmnMapCtrl;
            }
        }
        else
            rc = ERR_INVALID_PARAMETER;
    }
    else
        rc = ERR_INVALID_PARAMETER;

    /*LogRel("pspStubSmnUnmapByPtr: pv=%p -> rc=%d\n", pv, rc);*/
    return rc;
}


int pspSerialStubX86PhysMap(X86PADDR PhysX86Addr, bool fMmio, void **ppv)
{
    return pspStubX86PhysMap(&g_StubState, PhysX86Addr, fMmio, ppv);
}


int pspSerialStubX86PhysUnmapByPtr(void *pv)
{
    return pspStubX86PhysUnmapByPtr(&g_StubState, pv);
}


int pspSerialStubSmnUnmapByPtr(void *pv)
{
    return pspStubSmnUnmapByPtr(&g_StubState, pv);
}


int pspSerialStubSmnMap(SMNADDR SmnAddr, void **ppv)
{
    return pspStubSmnMap(&g_StubState, SmnAddr, ppv);
}


/**
 * Initializes the timekeeper using the 2nd timer which was so far only used by the on chip bootloader
 *
 * @returns Status code.
 * @param   pTimer                  Pointer to the timer to initalize.
 */
static int pspStubTimerInit(PPSPTIMER pTimer)
{
    int rc = TMInit(&pTimer->Tm);
    if (!rc)
    {
        /* Initialize the timer. */
        pTimer->cCnts       = 0;
        pTimer->cSubMsTicks = 0;
        *(volatile uint32_t *)(0x03010424 + 32) = 0;     /* Counter value. */
        *(volatile uint32_t *)(0x03010424)      = 0x101; /* This starts the timer. */
    }

    return rc;
}


/**
 * Handles any timing related stuff and advances the internal clock.
 *
 * @returns nothing.
 * @param   pTimer                  The global timer responsible for timekeeping.
 */
static void pspStubTimerHandle(PPSPTIMER pTimer)
{
    uint32_t cCnts = *(volatile uint32_t *)(0x03010424 + 32);
    uint32_t cTicksPassed = 0;

    /* Check how many ticks we advanced since the last check. */
    if (cCnts >= pTimer->cCnts)
        cTicksPassed = cCnts - pTimer->cCnts;
    else /* Wraparound. */
        cTicksPassed = cCnts + (0xffffffff - pTimer->cCnts) + 1;

    /*
     * Let the internal clock advance depending on the amount of milliseconds passed.
     *
     * 10ns granularity means 100 ticks per us -> 100 * 1000 ticks per ms.
     */
    if (cTicksPassed >= 100 * 1000)
    {
        uint32_t cMsPassed = cTicksPassed / (100 * 1000);
        TMTickMultiple(&pTimer->Tm, cMsPassed);
        cTicksPassed -= cMsPassed * (100 * 1000);
    }

    /* Check whether the remaining ticks added to the accumulated ones exceed 1ms. */
    cTicksPassed += pTimer->cSubMsTicks;
    if (cTicksPassed >= 100 * 1000)
    {
        TMTick(&pTimer->Tm);
        cTicksPassed -= 100 * 1000;
    }

    pTimer->cSubMsTicks = cTicksPassed;
    pTimer->cCnts       = cCnts;
}


/**
 * Returns the amount of milliseconds passed since power on/reset.
 *
 * @returns Number of milliseconds passed.
 * @param   pTimer                  The global timer.
 */
static uint32_t pspStubTimerGetMillies(PPSPTIMER pTimer)
{
    pspStubTimerHandle(pTimer);
    return TMGetMillies(&pTimer->Tm);
}


/**
 * Returns the global number of milliseconds passed.
 *
 * @returns Number of milliseconds passed.
 * @param   pThis                   The serial stub instance data.
 */
static inline uint32_t pspStubGetMillies(PPSPSTUBSTATE pThis)
{
    return pspStubTimerGetMillies(&pThis->Timer);
}


/**
 * Wait the given number of milli seconds.
 *
 * @returns nothing.
 * @param   pThis                   The serial stub instance data.
 * @param   cMillies                Number of milli seconds to wait.
 */
static void pspStubDelayMs(PPSPSTUBSTATE pThis, uint32_t cMillies)
{
    uint32_t tsStart = pspStubGetMillies(pThis);
    while (pspStubGetMillies(pThis) <= tsStart + cMillies);
}


void pspSerialStubDelayMs(uint32_t cMillies)
{
    pspStubDelayMs(&g_StubState, cMillies);
}


/**
 * Initializes the selected transport channel.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 */
static int pspStubTranspInit(PPSPSTUBSTATE pThis)
{
    PCPSPPDUTRANSPIF pTranspIf = NULL;

    if (pThis->fSpiMsgChan)
        pTranspIf = &g_SpiFlashTransp;
    else
        pTranspIf = &g_UartTransp;

    int rc = pTranspIf->pfnInit(&pThis->abTranspData[0], sizeof(pThis->abTranspData), &pThis->hPduTransp);
    if (rc == INF_SUCCESS)
        pThis->pIfTransp = pTranspIf;

    return rc;
}


/**
 * Returns the number of bytes available for reading.
 *
 * @returns Number of bytes available for reading.
 * @param   pThis                   The serial stub instance data.
 */
static size_t pspStubTranspPeek(PPSPSTUBSTATE pThis)
{
    return pThis->pIfTransp->pfnPeek(pThis->hPduTransp);
}


/**
 * Writes the given data to the underyling transport channel.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvBuf                   The data to write.
 * @param   cbWrite                 Number of bytes to write.
 */
static int pspStubTranspWrite(PPSPSTUBSTATE pThis, const void *pvBuf, size_t cbWrite)
{
    return pThis->pIfTransp->pfnWrite(pThis->hPduTransp, pvBuf, cbWrite, NULL /*pcbWritten*/);
}


/**
 * Reads data from the underyling transport channel.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  Number of bytes to read.
 */
static int pspStubTranspRead(PPSPSTUBSTATE pThis, void *pvBuf, size_t cbRead)
{
    return pThis->pIfTransp->pfnRead(pThis->hPduTransp, pvBuf, cbRead, NULL /*pcbRead*/);
}


/**
 * Marks begin of an access to the underyling transport channel.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  Number of bytes to read.
 */
static int pspStubTranspBegin(PPSPSTUBSTATE pThis)
{
    return pThis->pIfTransp->pfnBegin(pThis->hPduTransp);
}


/**
 * Marks end of an access to the underyling transport channel.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvBuf                   Where to store the read data.
 * @param   cbRead                  Number of bytes to read.
 */
static int pspStubTranspEnd(PPSPSTUBSTATE pThis)
{
    return pThis->pIfTransp->pfnEnd(pThis->hPduTransp);
}


/**
 * Sends the given PDU.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   rcReq                   Status code for a sresponse PDU.
 * @param   idCcd                   The CCD ID the PDU is designated for.
 * @param   enmPduRrnId             The Request/Response/Notification ID.
 * @param   pvPayload               Pointer to the PDU payload to send, optional.
 * @param   cbPayload               Size of the PDU payload in bytes.
 */
static int pspStubPduSend(PPSPSTUBSTATE pThis, int32_t rcReq, uint32_t idCcd, PSPSERIALPDURRNID enmPduRrnId, const void *pvPayload, size_t cbPayload)
{
    PSPSERIALPDUHDR PduHdr;
    PSPSERIALPDUFOOTER PduFooter;

    /* Initialize header and footer. */
    PduHdr.u32Magic           = PSP_SERIAL_PSP_2_EXT_PDU_START_MAGIC;
    PduHdr.u.Fields.cbPdu     = cbPayload;
    PduHdr.u.Fields.cPdus     = ++pThis->cPdusSent;
    PduHdr.u.Fields.enmRrnId  = enmPduRrnId;
    PduHdr.u.Fields.idCcd     = idCcd;
    PduHdr.u.Fields.rcReq     = rcReq;
    PduHdr.u.Fields.tsMillies = pspStubGetMillies(pThis);

    uint32_t uChkSum = 0;
    for (uint32_t i = 0; i < ELEMENTS(PduHdr.u.ab); i++)
        uChkSum += PduHdr.u.ab[i];

    const uint8_t *pbPayload = (const uint8_t *)pvPayload;
    for (size_t i = 0; i < cbPayload; i++)
        uChkSum += pbPayload[i];

    PduFooter.u32ChkSum = (0xffffffff - uChkSum) + 1;
    PduFooter.u32Magic  = PSP_SERIAL_PSP_2_EXT_PDU_END_MAGIC;

    /* Send everything, header first, then payload and footer last. */
    pspStubTranspBegin(pThis);
    int rc = pspStubTranspWrite(pThis, &PduHdr, sizeof(PduHdr));
    if (!rc && pvPayload && cbPayload)
        rc = pspStubTranspWrite(pThis, pvPayload, cbPayload);
    if (!rc)
        rc = pspStubTranspWrite(pThis, &PduFooter, sizeof(PduFooter));
    pspStubTranspEnd(pThis);

    return rc;
}


/**
 * Resets the PDU receive state machine.
 *
 * @returns nothing.
 * @param   pThis                   The serial stub instance data.
 */
static void pspStubPduRecvReset(PPSPSTUBSTATE pThis)
{
    pThis->enmPduRecvState = PSPSERIALPDURECVSTATE_HDR;
    pThis->cbPduRecvLeft   = sizeof(PSPSERIALPDUHDR);
    pThis->offPduRecv      = 0;
}


/**
 * Validates the given PDU header.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pHdr                    PDU header to validate.
 */
static int pspStubPduHdrValidate(PPSPSTUBSTATE pThis, PCPSPSERIALPDUHDR pHdr)
{
    if (pHdr->u32Magic != PSP_SERIAL_EXT_2_PSP_PDU_START_MAGIC)
        return -1;
    if (pHdr->u.Fields.cbPdu > sizeof(pThis->abPdu) - sizeof(PSPSERIALPDUHDR) - sizeof(PSPSERIALPDUFOOTER))
        return -1;
    if (   pHdr->u.Fields.enmRrnId < PSPSERIALPDURRNID_REQUEST_FIRST
        || pHdr->u.Fields.enmRrnId >= PSPSERIALPDURRNID_REQUEST_INVALID_FIRST)
        return -1;
    if (pHdr->u.Fields.cPdus != pThis->cPduRecvNext)
        return -1;
    if (pHdr->u.Fields.idCcd >= pThis->cCcds)
        return -1;

    return 0;
}


/**
 * Validates the complete PDU, the header was validated mostly at an earlier stage already.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pHdr                    The header of the PDU to validate.
 */
static int pspStubPduValidate(PPSPSTUBSTATE pThis, PCPSPSERIALPDUHDR pHdr)
{
    uint32_t uChkSum = 0;

    for (uint32_t i = 0; i < ELEMENTS(pHdr->u.ab); i++)
        uChkSum += pHdr->u.ab[i];

    uint8_t *pbPayload = (uint8_t *)(pHdr + 1);
    for (uint32_t i = 0; i < pHdr->u.Fields.cbPdu; i++)
        uChkSum += *pbPayload++;

    /* Check whether the footer magic and checksum are valid. */
    PCPSPSERIALPDUFOOTER pFooter = (PCPSPSERIALPDUFOOTER)pbPayload;
    if (   uChkSum + pFooter->u32ChkSum != 0
        || pFooter->u32Magic != PSP_SERIAL_EXT_2_PSP_PDU_END_MAGIC)
        return -1;

    return 0;
}


/**
 * Processes the current state and advances to the next one.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   ppPduRcvd               Where to store the pointer to the received complete PDU on success.
 */
static int pspStubPduRecvAdvance(PPSPSTUBSTATE pThis, PCPSPSERIALPDUHDR *ppPduRcvd)
{
    int rc = INF_SUCCESS;

    *ppPduRcvd = NULL;

    switch (pThis->enmPduRecvState)
    {
        case PSPSERIALPDURECVSTATE_HDR:
        {
            /* Validate header. */
            PCPSPSERIALPDUHDR pHdr = (PCPSPSERIALPDUHDR)&pThis->abPdu[0];

            int rc2 = pspStubPduHdrValidate(pThis, pHdr);
            if (!rc2)
            {
                /* No payload means going directly to the footer. */
                if (pHdr->u.Fields.cbPdu)
                {
                    pThis->enmPduRecvState = PSPSERIALPDURECVSTATE_PAYLOAD;
                    pThis->cbPduRecvLeft   = pHdr->u.Fields.cbPdu;
                }
                else
                {
                    pThis->enmPduRecvState = PSPSERIALPDURECVSTATE_FOOTER;
                    pThis->cbPduRecvLeft   = sizeof(PSPSERIALPDUFOOTER);
                }
            }
            else
            {
                /** @todo Send out of band error. */
                pspStubPduRecvReset(pThis);
            }
            break;
        }
        case PSPSERIALPDURECVSTATE_PAYLOAD:
        {
            /* Just advance to the next state. */
            pThis->enmPduRecvState = PSPSERIALPDURECVSTATE_FOOTER;
            pThis->cbPduRecvLeft   = sizeof(PSPSERIALPDUFOOTER);
            break;
        }
        case PSPSERIALPDURECVSTATE_FOOTER:
        {
            /* Validate the footer and complete PDU. */
            PCPSPSERIALPDUHDR pHdr = (PCPSPSERIALPDUHDR)&pThis->abPdu[0];

            rc = pspStubPduValidate(pThis, pHdr);
            if (!rc)
            {
                pThis->cPduRecvNext++;
                *ppPduRcvd = pHdr;
            }
            /** @todo Send out of band error. */
            /* Start receiving a new PDU in any case. */
            pspStubPduRecvReset(pThis);
            break;
        }
        default:
            rc = ERR_INVALID_STATE;
    }

    return rc;
}


/**
 * Waits for a PDU to be received or until the given timeout elapsed.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   ppPduRcvd               Where to store the pointer to the received complete PDU on success.
 * @param   cMillies                Amount of milliseconds to wait until a timeout is returned.
 */
static int pspStubPduRecv(PPSPSTUBSTATE pThis, PCPSPSERIALPDUHDR *ppPduRcvd, uint32_t cMillies)
{
    int rc = INF_SUCCESS;
    uint32_t tsStartMs = pspStubGetMillies(pThis);

    do
    {
        size_t cbAvail = pspStubTranspPeek(pThis);
        if (cbAvail)
        {
            /* Only read what is required for the current state. */
            /** @todo If the connection turns out to be unreliable we have to do a marker search first. */
            size_t cbThisRecv = MIN(cbAvail, pThis->cbPduRecvLeft);

            rc = pspStubTranspRead(pThis, &pThis->abPdu[pThis->offPduRecv], cbThisRecv);
            if (!rc)
            {
                pThis->offPduRecv    += cbThisRecv;
                pThis->cbPduRecvLeft -= cbThisRecv;

                /* Advance state machine and process the data if this state is complete. */
                if (!pThis->cbPduRecvLeft)
                {
                    rc = pspStubPduRecvAdvance(pThis, ppPduRcvd);
                    if (   !rc
                        && *ppPduRcvd != NULL)
                        break; /* We received a complete and valid PDU. */
                }
            }
        }
    } while (   !rc
             && (   pspStubGetMillies(pThis) - tsStartMs < cMillies
                 || cMillies == PSP_SERIAL_STUB_INDEFINITE_WAIT));

    if (   !rc
        && !*ppPduRcvd
        && tsStartMs + pspStubGetMillies(pThis) >= cMillies)
        rc = INF_TRY_AGAIN;

    return rc;
}


/**
 * Waits for a connect request PDU.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   cMillies                Amount of milliseconds to wait.
 */
static int pspStubCheckConnection(PPSPSTUBSTATE pThis, uint32_t cMillies)
{
    PCPSPSERIALPDUHDR pPdu;
    int rc = pspStubPduRecv(pThis, &pPdu, cMillies);
    if (rc == INF_SUCCESS)
    {
        /* We expect a connect request here. */
        if (pPdu->u.Fields.enmRrnId == PSPSERIALPDURRNID_REQUEST_CONNECT)
        {
            /* Send our response with some information. */
            PSPSERIALCONNECTRESP Resp;

            Resp.cbPduMax       = sizeof(pThis->abPdu);
            Resp.cbScratch      = sizeof(pThis->abScratch);
            Resp.PspAddrScratch = (PSPADDR)(uintptr_t)&pThis->abScratch[0];
            Resp.cSysSockets    = 1; /** @todo */
            Resp.cCcdsPerSocket = 1; /** @todo */
            Resp.au32Pad0       = 0;

            /* Reset the PDU counter. */
            pThis->cPdusSent     = 0;

            rc = pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, PSPSERIALPDURRNID_RESPONSE_CONNECT, &Resp, sizeof(Resp));
            if (!rc)
                pThis->fConnected = true;
        }
        /** @todo else Send out of band error. */
    }
    /* else Timeout -> return */

    return INF_SUCCESS;
}


/**
 * Reads/writes data in local PSP SRAM.
 *
 * @returns Stauts code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvPayload               PDU payload.
 * @param   cbPayload               Payload size in bytes.
 * @param   fWrite                  Flag whether this is rad or write request.
 */
static int pspStubPduProcessPspMemXfer(PPSPSTUBSTATE pThis, const void *pvPayload, size_t cbPayload, bool fWrite)
{
    int rc = INF_SUCCESS;
    PCPSPSERIALPSPMEMXFERREQ pReq = (PCPSPSERIALPSPMEMXFERREQ)pvPayload;

    if (cbPayload < sizeof(*pReq))
        return ERR_INVALID_PARAMETER;

    PSPSERIALPDURRNID enmResponse = PSPSERIALPDURRNID_INVALID;
    size_t cbXfer = pReq->cbXfer;
    const void *pvRespPayload = NULL;
    size_t cbResPayload = 0;
    if (fWrite)
    {
        enmResponse = PSPSERIALPDURRNID_RESPONSE_PSP_MEM_WRITE;
        void *pvDst = (void *)(uintptr_t)pReq->PspAddrStart;
        const void *pvSrc = (pReq + 1);
        memcpy(pvDst, pvSrc, cbXfer);
    }
    else
    {
        enmResponse   = PSPSERIALPDURRNID_RESPONSE_PSP_MEM_READ;
        pvRespPayload = (void *)(uintptr_t)pReq->PspAddrStart;
        cbResPayload  = cbXfer;
    }

    return pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, enmResponse, pvRespPayload, cbResPayload);
}


/**
 * Does a MMIO read/write of the given length.
 *
 * @returns nothing.
 * @param   pvDst               The destination.
 * @param   pvSrc               The source.
 * @param   cb                  Number of bytes to access.
 */
static void pspStubMmioAccess(void *pvDst, const void *pvSrc, size_t cb)
{
    switch (cb)
    {
        case 1:
            *(volatile uint8_t *)pvDst = *(volatile const uint8_t *)pvSrc;
            break;
        case 2:
            *(volatile uint16_t *)pvDst = *(volatile const uint16_t *)pvSrc;
            break;
        case 4:
            *(volatile uint32_t *)pvDst = *(volatile const uint32_t *)pvSrc;
            break;
        case 8:
            *(volatile uint64_t *)pvDst = *(volatile const uint64_t *)pvSrc;
            break;
        default:
            break;
    }
}


/**
 * Reads/writes data in local PSP MMIO space.
 *
 * @returns Stauts code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvPayload               PDU payload.
 * @param   cbPayload               Payload size in bytes.
 * @param   fWrite                  Flag whether this is rad or write request.
 */
static int pspStubPduProcessPspMmioXfer(PPSPSTUBSTATE pThis, const void *pvPayload, size_t cbPayload, bool fWrite)
{
    int rc = INF_SUCCESS;
    PCPSPSERIALPSPMEMXFERREQ pReq = (PCPSPSERIALPSPMEMXFERREQ)pvPayload;

    if (   cbPayload < sizeof(*pReq)
        || (   pReq->cbXfer != 1
            && pReq->cbXfer != 2
            && pReq->cbXfer != 4
            && pReq->cbXfer != 8))
        return ERR_INVALID_PARAMETER;

    PSPSERIALPDURRNID enmResponse = PSPSERIALPDURRNID_INVALID;
    size_t cbXfer = pReq->cbXfer;
    const void *pvRespPayload = NULL;
    uint8_t abRead[8];
    size_t cbResPayload = 0;
    const void *pvSrc = NULL;
    void *pvDst = NULL;
    if (fWrite)
    {
        enmResponse = PSPSERIALPDURRNID_RESPONSE_PSP_MMIO_WRITE;
        pvDst = (void *)(uintptr_t)pReq->PspAddrStart;
        pvSrc = (pReq + 1);
        pspStubMmioAccess(pvDst, (pReq + 1), cbXfer);
    }
    else
    {
        enmResponse   = PSPSERIALPDURRNID_RESPONSE_PSP_MMIO_READ;
        pvSrc = (void *)(uintptr_t)pReq->PspAddrStart;
        pvDst = &abRead[0];
        pspStubMmioAccess(&abRead[0], pvSrc, cbXfer);
        pvRespPayload = &abRead[0];
        cbResPayload  = cbXfer;
    }

    return pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, enmResponse, pvRespPayload, cbResPayload);
}


/**
 * Reads/writes data in the SMN address space.
 *
 * @returns Stauts code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvPayload               PDU payload.
 * @param   cbPayload               Payload size in bytes.
 * @param   fWrite                  Flag whether this is rad or write request.
 */
static int pspStubPduProcessPspSmnXfer(PPSPSTUBSTATE pThis, const void *pvPayload, size_t cbPayload, bool fWrite)
{
    PCPSPSERIALSMNMEMXFERREQ pReq = (PCPSPSERIALSMNMEMXFERREQ)pvPayload;

    if (   cbPayload < sizeof(*pReq)
        || (   pReq->cbXfer != 1
            && pReq->cbXfer != 2
            && pReq->cbXfer != 4
            && pReq->cbXfer != 8))
        return ERR_INVALID_PARAMETER;

    PSPSERIALPDURRNID enmResponse =   fWrite
                                    ? PSPSERIALPDURRNID_RESPONSE_PSP_SMN_WRITE
                                    : PSPSERIALPDURRNID_RESPONSE_PSP_SMN_READ;
    void *pvMap = NULL;
    int rc = pspStubSmnMap(pThis, pReq->SmnAddrStart, &pvMap);
    if (!rc)
    {
        const void *pvRespPayload = NULL;
        uint8_t abRead[8];
        size_t cbResPayload = 0;
        const void *pvSrc = NULL;
        void *pvDst = NULL;
        if (fWrite)
            pspStubMmioAccess(pvMap, (pReq + 1), pReq->cbXfer);
        else
        {
            pspStubMmioAccess(&abRead[0], pvMap, pReq->cbXfer);
            pvRespPayload = &abRead[0];
            cbResPayload  = pReq->cbXfer;
        }

        rc = pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, enmResponse, pvRespPayload, cbResPayload);
        pspStubSmnUnmapByPtr(pThis, pvMap);
    }
    else
        rc = pspStubPduSend(pThis, rc, 0 /*idCcd*/, enmResponse, NULL /*pvRespPayload*/, 0 /*cbRespPayload*/);

    return rc;
}


/**
 * Reads/writes data to normal memory in x86 address space.
 *
 * @returns Stauts code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvPayload               PDU payload.
 * @param   cbPayload               Payload size in bytes.
 * @param   fWrite                  Flag whether this is rad or write request.
 */
static int pspStubPduProcessPspX86MemXfer(PPSPSTUBSTATE pThis, const void *pvPayload, size_t cbPayload, bool fWrite)
{
    PCPSPSERIALX86MEMXFERREQ pReq = (PCPSPSERIALX86MEMXFERREQ)pvPayload;

    if (cbPayload < sizeof(*pReq))
        return ERR_INVALID_PARAMETER;

    PSPSERIALPDURRNID enmResponse =   fWrite
                                    ? PSPSERIALPDURRNID_RESPONSE_PSP_X86_MEM_WRITE
                                    : PSPSERIALPDURRNID_RESPONSE_PSP_X86_MEM_READ;
    void *pvMap = NULL;
    int rc = pspStubX86PhysMap(pThis, pReq->PhysX86Start, false /*fMmio*/, &pvMap);
    if (!rc)
    {
        size_t cbXfer = pReq->cbXfer;
        const void *pvRespPayload = NULL;
        size_t cbResPayload = 0;
        if (fWrite)
        {
            const void *pvSrc = (pReq + 1);
            memcpy(pvMap, pvSrc, cbXfer);
        }
        else
        {
            pvRespPayload = pvMap;
            cbResPayload  = cbXfer;
        }

        rc = pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, enmResponse, pvRespPayload, cbResPayload);
        pspStubX86PhysUnmapByPtr(pThis, pvMap);
    }
    else
        rc = pspStubPduSend(pThis, rc, 0 /*idCcd*/, enmResponse, NULL /*pvRespPayload*/, 0 /*cbRespPayload*/);

    return rc;
}


/**
 * Reads/writes data to MMIO in x86 address space.
 *
 * @returns Stauts code.
 * @param   pThis                   The serial stub instance data.
 * @param   pvPayload               PDU payload.
 * @param   cbPayload               Payload size in bytes.
 * @param   fWrite                  Flag whether this is rad or write request.
 */
static int pspStubPduProcessPspX86MmioXfer(PPSPSTUBSTATE pThis, const void *pvPayload, size_t cbPayload, bool fWrite)
{
    PCPSPSERIALX86MEMXFERREQ pReq = (PCPSPSERIALX86MEMXFERREQ)pvPayload;

    if (   cbPayload < sizeof(*pReq)
        || (   pReq->cbXfer != 1
            && pReq->cbXfer != 2
            && pReq->cbXfer != 4
            && pReq->cbXfer != 8))
        return ERR_INVALID_PARAMETER;

    PSPSERIALPDURRNID enmResponse =   fWrite
                                    ? PSPSERIALPDURRNID_RESPONSE_PSP_X86_MMIO_WRITE
                                    : PSPSERIALPDURRNID_RESPONSE_PSP_X86_MMIO_READ;
    void *pvMap = NULL;
    int rc = pspStubX86PhysMap(pThis, pReq->PhysX86Start, false /*fMmio*/, &pvMap);
    if (!rc)
    {
        const void *pvRespPayload = NULL;
        uint8_t abRead[8];
        size_t cbResPayload = 0;
        const void *pvSrc = NULL;
        void *pvDst = NULL;
        if (fWrite)
            pspStubMmioAccess(pvMap, (pReq + 1), pReq->cbXfer);
        else
        {
            pspStubMmioAccess(&abRead[0], pvMap, pReq->cbXfer);
            pvRespPayload = &abRead[0];
            cbResPayload  = pReq->cbXfer;
        }

        rc = pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, enmResponse, pvRespPayload, cbResPayload);
        pspStubX86PhysUnmapByPtr(pThis, pvMap);
    }
    else
        rc = pspStubPduSend(pThis, rc, 0 /*idCcd*/, enmResponse, NULL /*pvRespPayload*/, 0 /*cbRespPayload*/);

    return rc;
}


/**
 * Processes the given PDU.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 * @param   pPdu                    The PDU to process.
 */
static int pspStubPduProcess(PPSPSTUBSTATE pThis, PCPSPSERIALPDUHDR pPdu)
{
    int rc = INF_SUCCESS;

    switch (pPdu->u.Fields.enmRrnId)
    {
        case PSPSERIALPDURRNID_REQUEST_PSP_MEM_READ:
            rc = pspStubPduProcessPspMemXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, false /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_MEM_WRITE:
            rc = pspStubPduProcessPspMemXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, true /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_MMIO_READ:
            rc = pspStubPduProcessPspMmioXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, false /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_MMIO_WRITE:
            rc = pspStubPduProcessPspMmioXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, true /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_SMN_READ:
            rc = pspStubPduProcessPspSmnXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, false /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_SMN_WRITE:
            rc = pspStubPduProcessPspSmnXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, true /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_X86_MEM_READ:
            rc = pspStubPduProcessPspX86MemXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, false /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_X86_MEM_WRITE:
            rc = pspStubPduProcessPspX86MemXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, true /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_X86_MMIO_READ:
            rc = pspStubPduProcessPspX86MmioXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, false /*fWrite*/);
            break;
        case PSPSERIALPDURRNID_REQUEST_PSP_X86_MMIO_WRITE:
            rc = pspStubPduProcessPspX86MmioXfer(pThis, (pPdu + 1), pPdu->u.Fields.cbPdu, true /*fWrite*/);
            break;
        default:
            /* Should never happen as the ID was already checked during PDU validation. */
            break;
    }

    return rc;
}


static void pspStubMmioWrU32(PSPADDR PspAddrMmio, uint32_t uVal)
{
    pspStubMmioAccess((void *)PspAddrMmio, &uVal, sizeof(uint32_t));
}


static void pspStubMmioSetU32(PSPADDR PspAddrMmio, uint32_t fSet)
{
    uint32_t uVal;
    pspStubMmioAccess(&uVal, (void *)PspAddrMmio, sizeof(uint32_t));
    LogRel("pspStubMmioSetU32: PspAddrMmio=%#x fSet=%#x uVal=%#x\n",
           PspAddrMmio, fSet, uVal);
    uVal |= fSet;
    pspStubMmioAccess((void *)PspAddrMmio, &uVal, sizeof(uint32_t));
}


static void pspStubMmioClearU32(PSPADDR PspAddrMmio, uint32_t fClr)
{
    uint32_t uVal;
    pspStubMmioAccess(&uVal, (void *)PspAddrMmio, sizeof(uint32_t));
    LogRel("pspStubMmioClearU32: PspAddrMmio=%#x fClr=%#x uVal=%#x\n",
           PspAddrMmio, fClr, uVal);
    uVal &= ~fClr;
    pspStubMmioAccess((void *)PspAddrMmio, &uVal, sizeof(uint32_t));
}


static void pspStubMmioWaitOnClrU32(PSPADDR PspAddrMmio, uint32_t fWait)
{
    uint32_t uVal = 0;

    do
    {
        pspStubMmioAccess(&uVal, (void *)PspAddrMmio, sizeof(uint32_t));
    } while (uVal & fWait != 0);
}


static void pspStubSmnSetU32(PPSPSTUBSTATE pThis, SMNADDR SmnAddr, uint32_t fSet)
{
    void *pvMap = NULL;
    int rc = pspStubSmnMap(pThis, SmnAddr, &pvMap);
    if (!rc)
    {
        pspStubMmioSetU32((PSPADDR)pvMap, fSet);
        pspStubSmnUnmapByPtr(pThis, pvMap);
    }
}


static void pspStubSmnAndOrU32(PPSPSTUBSTATE pThis, SMNADDR SmnAddr, uint32_t fAnd, uint32_t fOr)
{
    void *pvMap = NULL;
    int rc = pspStubSmnMap(pThis, SmnAddr, &pvMap);
    if (!rc)
    {
        uint32_t uVal;
        pspStubMmioAccess(&uVal, (void *)pvMap, sizeof(uint32_t));
        uint32_t uValNew = (uVal & fAnd) | fOr;
        LogRel("pspStubSmnAndOrU32: SmnAddr=%#x fAnd=%#x fOr=%#x uVal=%#x uValNew=%#x\n",
               SmnAddr, fAnd, fOr, uVal, uValNew);
        pspStubMmioAccess((void *)pvMap, &uValNew, sizeof(uint32_t));
        pspStubSmnUnmapByPtr(pThis, pvMap);
    }
}


static SMNADDR s_aSmnAddrSet[] =
{
    0x2f00404,
    0x2f00c04,
    0x2f01004,
    0x2f01404,
    0x2f01804,
    0x2f01c04,
    0x2f02804,
    0x2f03404,
    0x2f04c04,
    0x2f05004,
    0x2f06004,
    0x2f07004,
    0x2f09004,
    0x2f09404,
    0x2f09c04,
    0x2f0b404,
    0x2f0b804,
    0x2f0c004,
    0x2f0c404,
    0x2f0c804,
    0x2f0cc04,
    0x2f0d004,
    0x2f0d404,
    0x2f10804,
    0x2f11804,
    0x2f11c04,
    0x2f14004,
    0x2f14404,
    0x2f14804,
    0x2f14c04,
    0x2f15004,
    0x2f15404,
    0x2f15804,
    0x2f15c04,
    0x2f16804,
    0x2f16c04,
    0x2f19004,
    0x2f1b004,
    0x2f1b404,
    0x2f1b804,
    0x2f1f004,
    0x2f20004,
    0x2f20404,
    0x2f24004,
    0x2f25804,
    0x2f25c04,
    0x2f2a004,
    0x2f2a804,
    0x2f2c004,
    0x2f2c404,
    0x2f36004,
    0x2f38004,
    0x2f38404,
    0x2f38804,
    0x2f38c04,
    0x2f39004,
    0x2f39404,
    0x2f39804,
    0x2f3fc04
};

static void pspStubInitHwUnkSmnRanges(PPSPSTUBSTATE pThis)
{
    for (uint32_t i = 0; i < ELEMENTS(s_aSmnAddrSet); i++)
        pspStubSmnAndOrU32(pThis, s_aSmnAddrSet[i], 0xffffffff, 0x4);
}


static void pspStubInitHw(PPSPSTUBSTATE pThis)
{
    pspStubMmioWaitOnClrU32(0x32000f0, 0x80000000);

    /* Missing something here... */

    pspStubInitHwUnkSmnRanges(pThis);
    pspStubMmioWrU32(0x3010618, 0);
    pspStubMmioWrU32(0x301061c, 0);
    pspStubMmioWrU32(0x3010620, 0);
    pspStubMmioWrU32(0x3010624, 0);

    pspStubSmnAndOrU32(pThis, 0x2d013ec, 0xffffffff, 0x14);
    pspStubSmnAndOrU32(pThis, 0x2d01344, 0xf7ffffff, 0x80000000);
    pspStubSmnAndOrU32(pThis, 0x17400404, 0xffffffff, 0x3ff);
    pspStubSmnAndOrU32(pThis, 0x17500404, 0xffffffff, 0x3ff);
    pspStubSmnAndOrU32(pThis, 0x0105a008, 0xfffffffe, 0);
    pspStubSmnAndOrU32(pThis, 0x0105b008, 0xfffffffe, 0);
    pspStubDelayMs(pThis, 100);
    pspStubSmnAndOrU32(pThis, 0x0105b008, 0xffffffff, 1);
    pspStubSmnAndOrU32(pThis, 0x0105a008, 0xffffffff, 1);

    pspStubDelayMs(pThis, 1000);
    pspStubMmioWrU32(0x301003c, 1);

    /* Clearing base addresses of x86 mapping control registers. */
    pspStubMmioWrU32(0x3230000, 0);
    pspStubMmioWrU32(0x3230004, 0);
    pspStubMmioWrU32(0x3230008, 0);
    pspStubMmioWrU32(0x323000c, 0);
    pspStubMmioWrU32(0x32303e0, 0);
    pspStubMmioWrU32(0x32304d8, 0);
}


static uint32_t pspStubGetPhysDieId(PPSPSTUBSTATE pThis)
{
    void *pvMap = NULL;
    int rc = pspStubSmnMap(pThis, 0x5a078, &pvMap);
    if (!rc)
    {
        uint32_t uVal;
        pspStubMmioAccess(&uVal, (void *)pvMap, sizeof(uint32_t));
        pspStubSmnUnmapByPtr(pThis, pvMap);
        return uVal & 0x3;
    }

    return 0xffffffff;
}


/**
 * The mainloop.
 *
 * @returns Status code.
 * @param   pThis                   The serial stub instance data.
 */
static int pspStubMainloop(PPSPSTUBSTATE pThis)
{
    int rc = INF_SUCCESS;

    LogRel("pspStubMainloop: Entering\n");

    /* Wait for someone to connect and send a beacon every once in a while. */
    while (   !pThis->fConnected
           && !rc)
    {
        PSPSERIALBEACONNOT Beacon;

        Beacon.cBeaconsSent = ++pThis->cBeaconsSent;
        Beacon.u32Pad0      = 0;
        rc = pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, PSPSERIALPDURRNID_NOTIFICATION_BEACON, &Beacon, sizeof(Beacon));
        if (!rc)
        {
            /* Spin for a second and check whether someone wants to connect to us. */
            rc = pspStubCheckConnection(pThis, 1000);
        }
    }

    if (   !rc
        && pThis->fConnected)
    {
        LogRel("pspStubMainloop: Connection established\n");

        /* Connected, main PDU receive function. */
        for (;;)
        {
            PCPSPSERIALPDUHDR pPdu;

            rc = pspStubPduRecv(pThis, &pPdu, PSP_SERIAL_STUB_INDEFINITE_WAIT);
            if (!rc)
                rc = pspStubPduProcess(pThis, pPdu);
        }
    }

    LogRel("pspStubMainloop: Exiting with %d\n", rc);
    return rc;
}


/**
 * Log flush callback.
 *
 * @returns nothing.
 * @param   pvUser              Opaque user data passed during logger creation.
 * @param   pbBuf               Buffer to flush.
 * @param   cbBuf               Number of bytes to flush.
 */
static void pspStubLogFlush(void *pvUser, uint8_t *pbBuf, size_t cbBuf)
{
    PPSPSTUBSTATE pThis = (PPSPSTUBSTATE)pvUser;

    if (pThis->fLogEnabled)
    {
        if (pThis->fEarlyLogOverSpi)
        {
            while (cbBuf >= 4)
            {
                *(volatile uint32_t *)(pThis->pvEarlySpiLog + off) = *(uint32_t *)pbBuf;
                off   += 4;
                cbBuf -= 4;
                pbBuf += 4;
            }

            if (cbBuf)
            {
                uint32_t uVal = 0;
                switch (cbBuf)
                {
                    case 3:
                        uVal = '\n' << 24 | pbBuf[2] << 16 | pbBuf[1] << 8 | pbBuf[0];
                        break;
                    case 2:
                        uVal = '\n' << 24 | '\n' << 16 | pbBuf[1] << 8 | pbBuf[0];
                        break;
                    case 1:
                        uVal = '\n' << 24 | '\n' << 16 | '\n' << 8 | pbBuf[0];
                    default:
                        break;
                }

                *(volatile uint32_t *)(pThis->pvEarlySpiLog + off) = uVal;
                off   += 4;
            }
        }
        else
            pspStubPduSend(pThis, INF_SUCCESS, 0 /*idCcd*/, PSPSERIALPDURRNID_NOTIFICATION_LOG_MSG, pbBuf, cbBuf);
    }
}


void ExcpUndefInsn(void)
{
    LogRel("ExcpUndefInsn:\n");
    for (;;);
}

void ExcpSwi(void)
{
    LogRel("ExcpSwi:\n");
    for (;;);
}


void ExcpPrefAbrt(void)
{
    LogRel("ExcpPrefAbrt:\n");
    for (;;);
}


void ExcpDataAbrt(void)
{
    LogRel("ExcpDataAbrt:\n");
    for (;;);
}


void ExcpIrq(void)
{
    LogRel("ExcpIrq:\n");
    for (;;);
}


void ExcpFiq(void)
{
    LogRel("ExcpFiq:\n");
    for (;;);
}


void main(void)
{
    /* Init the stub state and create the UART driver instances. */
    PPSPSTUBSTATE pThis = &g_StubState;

    off           = 0;

    pThis->cCcds                       = 1; /** @todo Determine the amount of available CCDs (can't be read from boot ROM service page at all times) */
    pThis->fConnected                  = false;
#ifdef PSP_SERIAL_STUB_SPI_MSG_CHAN
    pThis->fSpiMsgChan                 = true;
    pThis->fEarlyLogOverSpi            = false;
    pThis->fLogEnabled                 = false;
#else
    pThis->fSpiMsgChan                 = false;
    pThis->fEarlyLogOverSpi            = true;
    pThis->fLogEnabled                 = true;
#endif
    pThis->cBeaconsSent                = 0;
    pThis->cPdusSent                   = 0;
    pThis->cPduRecvNext                = 1;
    pspStubPduRecvReset(pThis);
    memset(&pThis->aX86MapSlots[0], 0, sizeof(pThis->aX86MapSlots));
    memset(&pThis->aSmnMapSlots[0], 0, sizeof(pThis->aSmnMapSlots));
    for (uint32_t i = 0; i < ELEMENTS(pThis->aX86MapSlots); i++)
        pThis->aX86MapSlots[i].PhysX86AddrBase = NIL_X86PADDR;

    pspStubSmnMap(pThis, 0xa0000000 + PSP_SERIAL_STUB_EARLY_SPI_LOG_OFF, &pThis->pvEarlySpiLog);

    /* Init the timer. */
    pspStubTimerInit(&pThis->Timer);
    pThis->pTm = &pThis->Timer.Tm;
    LOGLoggerInit(&pThis->Logger, pspStubLogFlush, pThis,
                  "PspSerialStub", pThis->pTm, LOG_LOGGER_INIT_FLAGS_TS_FMT_HHMMSS);
    LOGLoggerSetDefaultInstance(&pThis->Logger);

    /* Don't do anything if this is not the master PSP. */
    if (pspStubGetPhysDieId(pThis) != 0)
    {
        for (;;);
    }

    pspStubInitHw(pThis);

    LogRel("main: Hardware initialized\n");

    /* Initialize the data transport mechanism selected. */
    int rc = pspStubTranspInit(pThis);
    if (!rc)
    {
        /*pThis->fLogEnabled = true;*/
        pThis->fEarlyLogOverSpi = false;
        LogRel("main: Transport channel initialized -> starting mainloop\n");
        rc = pspStubMainloop(pThis);
    }

    LogRel("Serial stub is dead, waiting for reset...\n");
    /* Do not return on error. */
    for (;;);
}

