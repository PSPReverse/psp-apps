#ifndef psp_patch_addr_h
#define psp_patch_addr_h

#if PSP_VERSION == 1
#define PSP_FW_SVC_APP_CLEANUP_HANDLER_PATCH_ADDR (0x540)
#define PSP_FW_SVC_HANDLER_PATCH_ADDR             (0x544)
#define PSP_FW_TRAP_HANDLER_PATCH_ADDR            (0x53c)
#define PSP_MAP_X86_HOST_MEMORY_EX_ADDR           (0x6090)
#define PSP_SVC_HANDLER_ADDR                      (0x9518)
#define PSP_INV_MEM_ADDR                          (0x170c)
#else
# error "Unknown PSP version set"
#endif

#endif /* !psp_patch_addr_h */

