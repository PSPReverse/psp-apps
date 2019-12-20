#include <types.h>
#include <cdefs.h>
#include <svc.h>
#include <string.h>

#include "psp-svc-inject.h"

/** The address we patch the svc 0x6 replacement into. */
#define PSP_FW_SVC_6_PATCH_ADDR ((void *)0x12000)
#define PSP_FW_SVC_HANDLER_PATCH_ADDR ((void *)0x544)
#define PSP_FW_SVC_APP_CLEANUP_HANDLER_PATCH_ADDR ((void *)0x540)

#if 0
static void puts(const char *pszStr)
{
    size_t cchStr = strlen(pszStr);
    size_t cchAlign = cchStr & ~(uint32_t)0x3;
    size_t cchRem = cchStr - cchAlign;

    *(volatile uint32_t *)0x01AAB000 = 0xdeadc0de;

    while (cchAlign > 0)
    {
        *(volatile uint32_t *)0x01AAB000 = *(uint32_t *)pszStr;
        cchAlign -= sizeof(uint32_t);
        pszStr += sizeof(uint32_t);
    }

    switch (cchRem)
    {
        case 0:
            break;
        case 1:
            *(volatile uint32_t *)0x01AAB000 = (uint32_t)*pszStr;
            break;
        case 2:
            *(volatile uint32_t *)0x01AAB000 = (uint32_t)(pszStr[0] | (pszStr[1] << 8));
            break;
        case 3:
            *(volatile uint32_t *)0x01AAB000 = (uint32_t)(pszStr[0] | (pszStr[1] << 8) | (pszStr[2] << 16));
            break;
        default:
            do { } while (1);
    }

    *(volatile uint32_t *)0x01AAB000 = 0xc0dedead;
}

static void put_dword(uint32_t u32)
{
    *(volatile uint32_t *)0x01AAB000 = 0xdeadc0de;
    *(volatile uint32_t *)0x01AAB000 = u32;
    *(volatile uint32_t *)0x01AAB000 = 0xc0dedead;
}

static void dump_bytes(const uint8_t *pb, uint32_t cb)
{
    *(volatile uint32_t *)0x01AAB000 = 0xdeadc0de;
    for (unsigned i = 0; i < cb; i++)
        *(volatile uint32_t *)0x01AAB000 = *pb++ << 24;
    *(volatile uint32_t *)0x01AAB000 = 0xc0dedead;    
}
#endif

void main(void)
{
    /* Make sure the memory containing the page tables is cleared. */
    svc_invalidate_mem(SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE, 0, (void *)0x14000, 4096);
    svc_invalidate_mem(SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE, 0, (void *)0x13000, 4096);
    svc_invalidate_mem(SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE, 0, (void *)0x9000,  4096);

    volatile uint8_t *pbDst = (uint8_t *)PSP_FW_SVC_6_PATCH_ADDR;
    volatile uint8_t *pbSrc = (uint8_t *)psp_svc_inject;
    for (int i = 0; i < psp_svc_inject_length; i++)
        *pbDst++ = *pbSrc++;

    svc_invalidate_mem(SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE, 0, (void *)0x12000,  4096);

    /* Patch the branch to the syscall handler and override with our address. */
    *(volatile uint32_t *)PSP_FW_SVC_HANDLER_PATCH_ADDR = (uint32_t)PSP_FW_SVC_6_PATCH_ADDR;
    //*(volatile uint32_t *)PSP_FW_SVC_APP_CLEANUP_HANDLER_PATCH_ADDR = (uint32_t)PSP_FW_SVC_6_PATCH_ADDR;

    svc_dbg_print("Activating the svc interceptor\n"); /* DO NOT REMOVE OR EVERYTHING WILL FALL APART! */
#if 0
    svc_dbg_print("Overwriting x86 memory protection\n");
    *(uint8_t *)0x3360 = 0x4f;
    *(uint8_t *)0x3361 = 0xf4;
    *(uint8_t *)0x3362 = 0x90;
    *(uint8_t *)0x3363 = 0x33; /* mov.w r3, #0x12000 */

    *(uint8_t *)0x3364 = 0x03;
    *(uint8_t *)0x3365 = 0xf5;
    *(uint8_t *)0x3366 = 0x20;
    *(uint8_t *)0x3367 = 0x63; /* add.w r3, #0xa00 */

    *(uint8_t *)0x3368 = 0x03;
    *(uint8_t *)0x3369 = 0xf1;
    *(uint8_t *)0x336a = 0x69;
    *(uint8_t *)0x336b = 0x03; /* add.w r3, #0x69 */

    *(uint8_t *)0x336c = 0x18;
    *(uint8_t *)0x336d = 0x47; /* bx r3 */

    svc_dbg_print("Overwriting x86 memory protection finished\n");
    svc_invalidate_mem(SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE, 0, (void *)0x3000,  4096);
#endif
}
