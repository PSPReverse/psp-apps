#include "../../include/psp-fw/svc_id.h"

.section .text

/* void svc_exit(unsigned int code) */
.globl svc_exit;
svc_exit:
    svc #SVC_EXIT
    mov pc, lr
.type svc_exit, %function;

/* void svc_dbg_print(const char *pszStr) */
.globl svc_dbg_print;
svc_dbg_print:
    svc #SVC_DBG_PRINT
    mov pc, lr
.type svc_dbg_print, %function;

/* void svc_get_dbg_key(char* sp_dst, char* dst, unsigned int length) */
.globl svc_get_dbg_key;
svc_get_dbg_key:
    svc #SVC_GET_DBG_KEY
    mov pc, lr
.type svc_get_dbg_key, %function;

/* void svc_invalidate_mem(SVC_INV_MEM_OP enmOp, uint32_t fInsnMem, void *pvStart, uint32_t cbMem) */
.globl svc_invalidate_mem;
svc_invalidate_mem:
    svc #SVC_CACHE_FLUSH
    mov pc, lr
.type svc_invalidate_mem, %function;

/* void *svc_get_state_buffer(size_t cbBuf) */
.globl svc_get_state_buffer;
svc_get_state_buffer:
    svc #SVC_GET_STATE_BUFFER
    mov pc, lr
.type svc_get_state_buffer, %function;

/* uint32_t svc_x86_host_memory_copy_from(X86PADDR PhysX86AddrSrc, void *pvDst, size_t cbCopy) */
.globl svc_x86_host_memory_copy_from;
svc_x86_host_memory_copy_from:
    svc #SVC_X86_HOST_MEMORY_COPY_FROM
    mov pc, lr
.type svc_x86_host_memory_copy_from, %function;

/* void *svc_x86_host_memory_map(X86PADDR PhysX86AddrMap, uint32_t enmMemType) */
.globl svc_x86_host_memory_map;
svc_x86_host_memory_map:
    svc #SVC_X86_HOST_MEMORY_MAP
    mov pc, lr
.type svc_x86_host_memory_map, %function;

/* uint32_t svc_x86_host_memory_unmap(void *pvMapped) */
.globl svc_x86_host_memory_unmap;
svc_x86_host_memory_unmap:
    svc #SVC_X86_HOST_MEMORY_UNMAP
    mov pc, lr
.type svc_x86_host_memory_unmap, %function;

/* uint32_t svc_query_two_64bit_values(uint64_t *pu64Val1, uint64_t *pu64Val2) */
.globl svc_query_two_64bit_values;
svc_query_two_64bit_values:
    svc #SVC_QUERY_TWO_64BIT_VALS
    mov pc, lr
.type svc_query_two_64bit_values, %function;

/* uint16_t svc_x86_host_memory_copy_to_psp(PPSPX86MEMCOPYREQ pReq) */
.globl svc_x86_host_memory_copy_to_psp;
svc_x86_host_memory_copy_to_psp:
    svc #SVC_X86_HOST_COPY_TO_PSP
    mov pc, lr
.type svc_x86_host_memory_copy_to_psp, %function;

/* uint16_t svc_x86_host_memory_copy_from_psp(PPSPX86MEMCOPYREQ pReq) */
.globl svc_x86_host_memory_copy_from_psp;
svc_x86_host_memory_copy_from_psp:
    svc #SVC_X86_HOST_COPY_FROM_PSP
    mov pc, lr
.type svc_x86_host_memory_copy_from_psp, %function;

/* uint32_t svc_call_other_psp(uint32_t idCcx, void *pvReq, size_t cbReq) */
.globl svc_call_other_psp;
svc_call_other_psp:
    svc #SVC_CALL_OTHER_PSP
    mov pc, lr
.type svc_call_other_psp, %function;

/* void * svc_smn_map_ex(uint32_t u32SmnAddr, uint32_t idCcxTgt) */
.globl svc_smn_map_ex;
svc_smn_map_ex:
    svc #SVC_SMN_MAP_ADDR_EX
    mov pc, lr
.type svc_smn_map_ex, %function;

/* uint32_t svc_smn_unmap(void *pvUnmap) */
.globl svc_smn_unmap;
svc_smn_unmap:
    svc #SVC_SMN_UNMAP_ADDR
    mov pc, lr
.type svc_smn_unmap, %function;

.globl svc_unknown_id_16;
svc_unknown_id_16:
    svc #SVC_UNKNOWN_ID_16
    mov pc, lr
.type svc_unknown_id_16, %function

.globl svc_unknown_id_4b;
svc_unknown_id_4b:
    svc #SVC_UNKNOWN_ID_4b
    mov pc, lr
.type svc_unknown_id_4b, %function


/* void *svc_injected_map_x86_host_memory_ex(X86PADDR PhysX86AddrMap, uint32_t enmType, uint32_t fFlags) */
.globl svc_injected_map_x86_host_memory_ex;
svc_injected_map_x86_host_memory_ex:
    svc #SVC_INJECTED_MAP_X86_HOST_MEMORY_EX
    mov pc, lr
.type svc_injected_map_x86_host_memory_ex, %function;

/* void svc_log_char_buf(const char *pbBuf, size_t cchBuf) */
.globl svc_log_char_buf;
svc_log_char_buf:
    svc #SVC_LOG_CHAR_BUF
    mov pc, lr
.type svc_log_char_buf, %function;

/* uint32_t svc_dbg_marker_1(void) */
.globl svc_dbg_marker_1;
svc_dbg_marker_1:
    svc #SVC_INJECTED_DBG_MARKER_1
    mov pc, lr
.type svc_dbg_marker_1, %function;

/* uint32_t svc_dbg_marker_2(void) */
.globl svc_dbg_marker_2;
svc_dbg_marker_2:
    svc #SVC_INJECTED_DBG_MARKER_2
    mov pc, lr
.type svc_dbg_marker_2, %function;

/* uint32_t svc_dbg_marker_3(void) */
.globl svc_dbg_marker_3;
svc_dbg_marker_3:
    svc #SVC_INJECTED_DBG_MARKER_3
    mov pc, lr
.type svc_dbg_marker_3, %function;

/* uint32_t svc_dbg_marker_4(void) */
.globl svc_dbg_marker_4;
svc_dbg_marker_4:
    svc #SVC_INJECTED_DBG_MARKER_4
    mov pc, lr
.type svc_dbg_marker_4, %function;

/* uint32_t svc_dbg_marker_5(void) */
.globl svc_dbg_marker_5;
svc_dbg_marker_5:
    svc #SVC_INJECTED_DBG_MARKER_5
    mov pc, lr
.type svc_dbg_marker_5, %function;

/* uint32_t svc_template(uint32_t u32R0, uint32_t u32R1, uint32_t u32R2, uint32_t u32R3) */
.globl svc_template;
svc_template:
    svc #SVC_CALL_INVALID
    mov pc, lr
.type svc_template, %function;

