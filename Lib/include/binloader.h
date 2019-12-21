/**
 * Binary loader interface.
 */
#ifndef __include_binloader_h
#define __include_binloader_h

#include <types.h>
#include <log.h>

/** Load address of the payload. */
#define BIN_LOADER_LOAD_ADDR  (0x1d000)

/**
 * Helper struct passed to the entry point containing some useful data about the environment.
 */
typedef struct BINLDRHLP
{
    /** The CCD ID of the invoked PSP. */
    uint32_t            idCcd;
    /** Number of CCDs in the system. */
    uint32_t            cCcds;
    /** The passed pCmdBuf argument in main. */
    void                *pvCmdBuf;
    /** Logging function. */
    void                (*pfnLog)(const char *pszFmt, ...);
} BINLDRHLP;
typedef BINLDRHLP *PBINLDRHLP;
typedef const BINLDRHLP *PCBINLDRHLP;

/** Entry point for the binary loader payload. */
typedef void *FNBINLOADENTRY(void *pvState, PCBINLDRHLP pHlp, X86PADDR PhysX86AddrCmdBuf, uint32_t idCmd, uint8_t fFirstRun);
typedef FNBINLOADENTRY *PFNBINLOADENTRY;

#endif
