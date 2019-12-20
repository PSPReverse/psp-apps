/**
 * PSP privileged firmware part interfaces and defined structures.
 */
#ifndef __include_psp_h
#define __include_psp_h

#include <types.h>

/**
 * PSP register frame for the syscall handler.
 */
typedef struct PSPREGFRAME
{
    uint32_t auGprs[13];
} PSPREGFRAME;

#define REG_IDX_LR 12

/** Pointer to a register frame. */
typedef PSPREGFRAME *PPSPREGFRAME;

#endif
