#ifndef __include_svc_h
#define __include_svc_h

#include <types.h>
#include <psp-fw/svc_id.h>

typedef enum SVC_INV_MEM_OP
{
	SVC_INV_MEM_OP_CLEAN                = 0x0,
	SVC_INV_MEM_OP_INVALIDATE           = 0x1,
	SVC_INV_MEM_OP_CLEAN_AND_INVALIDATE = 0x2,
	SVC_INV_MEM_OP_32BIT_HACK           = 0x7fffffff /**< Blow the type up to 32bit. */
} SVC_INV_MEM_OP;

typedef enum PSP_X86_MEM_TYPE
{
    PSP_X86_MEM_TYPE_UNKNOWN_1  = 0x1,
    PSP_X86_MEM_TYPE_UNKNOWN_4  = 0x4,
    PSP_X86_MEM_TYPE_UNKNOWN_6  = 0x6,
    PSP_X86_MEM_TYPE_32BIT_HACK = 0x7fffffff
} PSP_X86_MEM_TYPE;

typedef struct PSPX86MEMCOPYREQ
{
    X86PADDR                PhysX86AddrSrc;
    void                    *pvDst;
    uint32_t                cbCopy;
    PSP_X86_MEM_TYPE        enmMemType;
} PSPX86MEMCOPYREQ;
typedef PSPX86MEMCOPYREQ *PPSPX86MEMCOPYREQ;

#define svc(code) asm volatile ("svc %[immediate]"::[immediate] "I" (code))

void svc_exit(unsigned int code);

void svc_dbg_print(const char *pszStr);

void svc_get_dbg_key(char* sp_dst, char* dst, unsigned int length);

void svc_invalidate_mem(SVC_INV_MEM_OP enmOp, uint32_t fInsnMem, void *pvStart, uint32_t cbMem);

void *svc_get_state_buffer(size_t cbBuf);

uint32_t svc_x86_host_memory_copy_from(X86PADDR PhysX86AddrSrc, void *pvDst, size_t cbCopy);

void *svc_x86_host_memory_map(X86PADDR PhysX86AddrMap, uint32_t enmMemType);

uint32_t svc_x86_host_memory_unmap(void *pvMapped);

uint32_t svc_query_two_64bit_values(uint64_t *pu64Val1, uint64_t *pu64Val2);

uint16_t svc_x86_host_memory_copy_to_psp(PPSPX86MEMCOPYREQ pReq);

uint16_t svc_x86_host_memory_copy_from_psp(PPSPX86MEMCOPYREQ pReq);

uint32_t svc_call_other_psp(uint32_t idCcx, void *pvReq, size_t cbReq);

void * svc_smn_map_ex(uint32_t u32SmnAddr, uint32_t idCcxTgt);

uint32_t svc_smn_unmap(void *pvUnmap);

/** Own injected syscalls. */

void *svc_injected_map_x86_host_memory_ex(X86PADDR PhysX86AddrMap, uint32_t enmType, uint32_t fFlags);

void svc_log_char_buf(const char *pbBuf, size_t cchBuf);

uint32_t svc_dbg_marker_1(void);
uint32_t svc_dbg_marker_2(void);
uint32_t svc_dbg_marker_3(void);
uint32_t svc_dbg_marker_4(void);
uint32_t svc_dbg_marker_5(void);

uint32_t svc_template(uint32_t u32R0, uint32_t u32R1, uint32_t u32R2, uint32_t u32R3);

#endif
