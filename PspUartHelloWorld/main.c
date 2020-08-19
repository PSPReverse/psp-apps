#include <uart.h>
#include <x86-map.h>
#include <smn-map.h>
#include <types.h>
#include <io.h>
#include <string.h>
#include <common/status.h>
#include <err.h>

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

/* TODO: Move to dedicated platform file */
#if 1
static void pspSerialSuperIoInit(void)
{
    pspX86MmioWriteU32(0xfffe000a3048, 0x0020ff00);
    pspX86MmioWriteU32(0xfffe000a30d0, 0x08fdff86);
    pspX86MmioWriteU8(0xfed81e77, 0x27);
    pspX86MmioWriteU32(0xfec20040, 0x0);
    pspX86MmioWriteU32(0xfffe000a3044, 0xc0);
    pspX86MmioWriteU32(0xfffe000a3048, 0x20ff07);
    pspX86MmioWriteU32(0xfffe000a3064, 0x1640);
    pspX86MmioWriteU32(0xfffe000a3000, 0xffffff00);
    pspX86MmioWriteU32(0xfffe000a30a0, 0xfec10002);
    pspX86MmioWriteU32(0xfed80300,     0xe3020b11);
    pspX86MmioWriteU8(0xfffdfc000072, 0x6);
    pspX86MmioWriteU8(0xfffdfc000072, 0x7);
    pspSmnWrU32(0x2dc58d0, 0x0c7c17cf);
    pspX86MmioWriteU32(0xfffe000a3044, 0xc0);
    pspX86MmioWriteU32(0xfffe000a3048, 0x20ff07);
    pspX86MmioWriteU32(0xfffe000a3064, 0x1640);
#if 0
    pspX86MmioWriteU8(0xfffdfc00002e, 0x87);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x24);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x00);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x10);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x87);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x55);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x23);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x40);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x40);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x07);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x61);
    pspX86MmioWriteU8(0xfffdfc00002f, 0xf8);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x60);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x03);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x30);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x01);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x02);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x02);
#else 
    pspX86MmioWriteU8(0xfffdfc00002e, 0xa5);
    pspX86MmioWriteU8(0xfffdfc00002e, 0xa5);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x7);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x2);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x61);
    pspX86MmioWriteU8(0xfffdfc00002f, 0xf8);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x60);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x3);
    pspX86MmioWriteU8(0xfffdfc00002e, 0x30);
    pspX86MmioWriteU8(0xfffdfc00002f, 0x1);
    pspX86MmioWriteU8(0xfffdfc00002e, 0xaa);
#endif
}

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
#endif

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

int main(void) {

    asm volatile("dsb #0xf\n"
                 "isb #0xf\n"
                 "cpsid if\n": : :"memory");
  char a = 'A';
  PSPUART uart;
  PSPPDUTRANSPINT ptrans;
  ptrans.IfIoDev.pfnRegWrite = pspUartRegWrite;
  ptrans.IfIoDev.pfnRegRead = pspUartRegRead;
  ptrans.pvUart = NULL;
  pspX86MapInit();
  pspSmnMapInit();

  /* pspSerialSuperIoInit(); */
  pspX86PhysMap(0xfffdfc000080, true, (void**)&(ptrans.pvUart));
  *(uint8_t*)ptrans.pvUart = 0x1;


  /* int rc; */

  PSPUartCreate(&uart, &(ptrans.IfIoDev));
  PSPUartParamsSet(&uart, 115200, PSPUARTDATABITS_8BITS, PSPUARTPARITY_NONE, PSPUARTSTOPBITS_1BIT);

  PSPUartWrite(&uart, "Hello World\n", sizeof("Hello World\n"), NULL);

  return 0;
}
