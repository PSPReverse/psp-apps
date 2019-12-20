/**
 * Definitions for the svc tracer (producer and consumer part).
 */
#ifndef __include_svctrace_h
#define __include_svctrace_h

#if !defined(SVC_TRACE_FMT_STRING) && !defined(SVC_TRACE_FMT_BINARY) && !defined(SVC_TRACE_FMT_STRING_COMPACT)
# error "Either SVC_TRACE_FMT_STRING or SVC_TRACE_FMT_BINARY or SVC_TRACE_FMT_STRING_COMPACT needs to be defined"
#endif

#define SVC_PSP_DETECT_MARKER_1   0xd00f
#define SVC_PSP_DETECT_MARKER_2   0xf00d
#define SVC_PSP_DETECT_MARKER_3   0xfeed
#define SVC_PSP_DETECT_MARKER_4   0xfade
#define SVC_PSP_LOG_MARKER_STRING 0xdead
#define SVC_PSP_LOG_MARKER_BINARY 0xc0de

#define SVC_LOG_ARG_TYPE_UINT8    0x0
#define SVC_LOG_ARG_TYPE_INT8     0x1
#define SVC_LOG_ARG_TYPE_UINT16   0x2
#define SVC_LOG_ARG_TYPE_INT16    0x3
#define SVC_LOG_ARG_TYPE_UINT32   0x4
#define SVC_LOG_ARG_TYPE_INT32    0x5
#define SVC_LOG_ARG_TYPE_UINT64   0x6
#define SVC_LOG_ARG_TYPE_INT64    0x7
#define SVC_LOG_ARG_TYPE_PSP_ADDR 0x8
#define SVC_LOG_ARG_TYPE_X86_ADDR 0x9
#define SVC_LOG_ARG_TYPE_STRING   0xa
#define SVC_LOG_ARG_TYPE_MASK     0xf
#define SVC_LOG_ARG_TYPE_GET(a_Flags) ((a_Flags) & SVC_LOG_ARG_LOG_F_MASK)

#define SVC_LOG_ARG_LOG_F_DEF     0x0
#define SVC_LOG_ARG_LOG_F_HEX     0x1
#define SVC_LOG_ARG_LOG_F_EXIT    0x2
#define SVC_LOG_ARG_LOG_F_CONTENT 0x4 /**< Logs content of the memory the pointer points to. */
#define SVC_LOG_ARG_LOG_F_SHIFT   8
#define SVC_LOG_ARG_LOG_F_MASK    0xf
#define SVC_LOG_ARG_LOG_F_SET(a_Flags) ((a_Flags) << SVC_LOG_ARG_LOG_F_SHIFT)
#define SVC_LOG_ARG_LOG_F_GET(a_Flags) (((a_Flags) >> SVC_LOG_ARG_LOG_F_SHIFT) & SVC_LOG_ARG_LOG_F_MASK)

#define SVC_LOG_ARG_BUILD(a_Type, a_Flags) ((a_Type) | SVC_LOG_ARG_LOG_F_SET(a_Flags))

typedef struct SVCARG
{
#ifdef SVC_TRACE_FMT_STRING
    const char *pszName;
#endif
    uint16_t   fArg;
    uint16_t   cbPtr;
} SVCARG;

typedef struct SVCENTRY
{
#ifdef SVC_TRACE_FMT_STRING
    const char       *pszName;
#endif
    const SVCARG     *paArgs;
    uint32_t         cArgs;
} SVCENTRY;

#ifdef SVC_TRACE_FMT_STRING
# define SVC_TRACE_ARG_INIT_DEF(a_pszName, a_ArgType, a_ArgFlags) { a_pszName, SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), 0 }
# define SVC_TRACE_ARG_INIT_MEMDUMP(a_pszName, a_ArgType, a_ArgFlags, a_cb) { a_pszName, SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), a_cb }

# define SVC_TRACE_SVC_INIT_DEF(a_pszName, a_aArgs) { a_pszName, &a_aArgs[0], ELEMENTS(a_aArgs) }
# define SVC_TRACE_SVC_INIT_ARG_COUNT(a_pszName, a_aArgs, a_cArgs) { a_pszName, &a_aArgs[0], a_cArgs }
# define SVC_TRACE_SVC_INIT_NO_ARGS(a_pszName) { a_pszName, NULL, 0 }
# define SVC_TRACE_SVC_INIT_UNKNOWN { NULL, NULL, 0 }

#elif defined(SVC_TRACE_FMT_STRING_COMPACT)

# define SVC_TRACE_ARG_INIT_DEF(a_pszName, a_ArgType, a_ArgFlags) { SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), 0 }
# define SVC_TRACE_ARG_INIT_MEMDUMP(a_pszName, a_ArgType, a_ArgFlags, a_cb) { SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), a_cb }

# define SVC_TRACE_SVC_INIT_DEF(a_pszName, a_aArgs) { &a_aArgs[0], ELEMENTS(a_aArgs) }
# define SVC_TRACE_SVC_INIT_ARG_COUNT(a_pszName, a_aArgs, a_cArgs) { &a_aArgs[0], a_cArgs }
# define SVC_TRACE_SVC_INIT_NO_ARGS(a_pszName) { NULL, 0 }
# define SVC_TRACE_SVC_INIT_UNKNOWN { NULL, 0 }

#elif defined(SVC_TRACE_FMT_BINARY)

# define SVC_TRACE_ARG_INIT_DEF(a_pszName, a_ArgType, a_ArgFlags) { SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), 0 }
# define SVC_TRACE_ARG_INIT_MEMDUMP(a_pszName, a_ArgType, a_ArgFlags, a_cb) { SVC_LOG_ARG_BUILD(a_ArgType, a_ArgFlags), a_cb }

# define SVC_TRACE_SVC_INIT_DEF(a_pszName, a_aArgs) { &a_aArgs[0], ELEMENTS(a_aArgs) }
# define SVC_TRACE_SVC_INIT_ARG_COUNT(a_pszName, a_aArgs, a_cArgs) { &a_aArgs[0], a_cArgs }
# define SVC_TRACE_SVC_INIT_NO_ARGS(a_pszName) { NULL, 0 }
# define SVC_TRACE_SVC_INIT_UNKNOWN { NULL, 0 }
#else
# error "Impossible!"
#endif

static const SVCARG g_aSvcAppLoadFromFlashArgs[] =
{
    SVC_TRACE_ARG_INIT_DEF("enmId",    SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("pUsrDst",  SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("cbUsrDst", SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcSmnMapAddrArgs[] =
{
    SVC_TRACE_ARG_INIT_DEF("u32SmnAddr", SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("u32Unknown", SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcSmnUnmapArgs[] =
{
    SVC_TRACE_ARG_INIT_DEF("pUsrSmnAddr", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcDbgLog[] =
{
    SVC_TRACE_ARG_INIT_DEF("pszLog", SVC_LOG_ARG_TYPE_STRING, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86MapPAddr[] =
{
    SVC_TRACE_ARG_INIT_DEF("PhysX86Addr", SVC_LOG_ARG_TYPE_X86_ADDR, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86UnmapPAddr[] =
{
    SVC_TRACE_ARG_INIT_DEF("pUsrMap", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86PAddrRead[] =
{
    SVC_TRACE_ARG_INIT_DEF("PhysX86Addr", SVC_LOG_ARG_TYPE_X86_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("pUsrDst",     SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("cbCopy",      SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86StsWrite[] =
{
    SVC_TRACE_ARG_INIT_DEF("PhysX86Addr", SVC_LOG_ARG_TYPE_X86_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("u32Val",      SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("cbCopy",      SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcPerformRsa[] =
{
    SVC_TRACE_ARG_INIT_DEF("pReqR3", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcWaitTicks[] =
{
    SVC_TRACE_ARG_INIT_DEF("cTicks", SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcDetermineBootMode[] =
{
    SVC_TRACE_ARG_INIT_MEMDUMP("pu32BootModeR3", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF | SVC_LOG_ARG_LOG_F_EXIT, 4)
};

static const SVCARG g_aSvcCopy32BBufToSvc[] =
{
    SVC_TRACE_ARG_INIT_DEF("pUsrMap", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86MapPAddrEx[] =
{
    SVC_TRACE_ARG_INIT_DEF("PhysX86Addr", SVC_LOG_ARG_TYPE_X86_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("enmMemType",  SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86CopyReq[] =
{
    SVC_TRACE_ARG_INIT_DEF("pReq", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcX86PAddrWrite[] =
{
    SVC_TRACE_ARG_INIT_DEF("pReq", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcSmuRsv[] =
{
    SVC_TRACE_ARG_INIT_DEF("PhysX86Addr", SVC_LOG_ARG_TYPE_X86_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("u32Unknown",  SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("u32Unknown2", SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcSmuSendMsg[] =
{
    SVC_TRACE_ARG_INIT_DEF("idMsg",      SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("uArg0",      SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_MEMDUMP("pu32SmuRet", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX | SVC_LOG_ARG_LOG_F_EXIT, 4)
};

static const SVCARG g_aSvcX86QuerySmm[] =
{
    SVC_TRACE_ARG_INIT_MEMDUMP("pPhysX86AddrSmmStart", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX | SVC_LOG_ARG_LOG_F_EXIT, 8),
    SVC_TRACE_ARG_INIT_MEMDUMP("pcbSmm",               SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX | SVC_LOG_ARG_LOG_F_EXIT, 4)
};

static const SVCARG g_aSvcMemInvClean[] =
{
    SVC_TRACE_ARG_INIT_DEF("enmOp",     SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("fData",     SVC_LOG_ARG_TYPE_UINT8,    SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("pUsrStart", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_DEF),
    SVC_TRACE_ARG_INIT_DEF("cbRegion",  SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvcCallSlvPsp[] =
{
    SVC_TRACE_ARG_INIT_DEF(    "idCcx",    SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_MEMDUMP("pReq",     SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX,  4/*24*/),
    SVC_TRACE_ARG_INIT_DEF(    "cbReq",    SVC_LOG_ARG_TYPE_UINT32,   SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvc0x33[] =
{
    SVC_TRACE_ARG_INIT_DEF("pvUsrUnk",  SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX),
    SVC_TRACE_ARG_INIT_DEF("cbUnk",     SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvc0x35[] =
{
    SVC_TRACE_ARG_INIT_DEF("pvUsrUnk",  SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcCcpReq[] =
{
    SVC_TRACE_ARG_INIT_DEF("pCcpReq",  SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCARG g_aSvcAsidSetMinNonEs[] =
{
    SVC_TRACE_ARG_INIT_DEF("uAsidMinNonEs",  SVC_LOG_ARG_TYPE_UINT32, SVC_LOG_ARG_LOG_F_DEF)
};

static const SVCARG g_aSvc0x41[] =
{
    SVC_TRACE_ARG_INIT_DEF("pvUsrUnk", SVC_LOG_ARG_TYPE_PSP_ADDR, SVC_LOG_ARG_LOG_F_HEX)
};

static const SVCENTRY g_aSvcEntries[] = 
{
    SVC_TRACE_SVC_INIT_NO_ARGS(  "psp_svc_app_cleanup"),                                                                  /**< 0x00 */
    SVC_TRACE_SVC_INIT_NO_ARGS(  "psp_svc_app_init"),                                                                     /**< 0x01 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_app_load_from_flash", g_aSvcAppLoadFromFlashArgs),                              /**< 0x02 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_smn_map_addr_ex",     g_aSvcSmnMapAddrArgs),                                    /**< 0x03 */
    SVC_TRACE_SVC_INIT_ARG_COUNT("psp_svc_smn_map_addr",        g_aSvcSmnMapAddrArgs, ELEMENTS(g_aSvcSmnMapAddrArgs) - 1),/**< 0x04 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_smn_unmap",           g_aSvcSmnUnmapArgs),                                      /**< 0x05 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_dbg_log",             g_aSvcDbgLog),                                            /**< 0x06 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_paddr_map",       g_aSvcX86MapPAddr),                                       /**< 0x07 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_paddr_unmap",     g_aSvcX86UnmapPAddr),                                     /**< 0x08 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_paddr_read",      g_aSvcX86PAddrRead),                                      /**< 0x09 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_sts_write",       g_aSvcX86StsWrite),                                       /**< 0x0a */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x0b Invalidating, cleaning memory. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x0c CCP crypto interface. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_perform_rsa",         g_aSvcPerformRsa),                                        /**< 0x0d Performs an RSA operation. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x0e Unknown, interfaces the CCP and uses same handler as svc 0x36. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x0f Unknown, looks like an SHA256 operation. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x10 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x11 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x12 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x13 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x14 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x15 Unknown, related to BHD parsing. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x16 Unknown, related to BHD parsing. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x17 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x18 Inter die communication (MCM) related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x19 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x1a Unknown, not provided by firmware. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_wait_ticks",          g_aSvcWaitTicks),                                         /**< 0x1b */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_determine_boot_mode", g_aSvcDetermineBootMode),                                 /**< 0x1c Determines the boot mode. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x1d Unknown, access the PSP HMAC related maybe. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x1e Unknown, loads something from flash maybe and does some hmac. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x1f Unknown, loads something from flash. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_copy_32byte_buf_to_svc",  g_aSvcCopy32BBufToSvc),                                   /**< 0x20 */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x21 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x22 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x23 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x24 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_paddr_map_ex",    g_aSvcX86MapPAddrEx),                                     /**< 0x25 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_copy_req",        g_aSvcX86CopyReq),                                        /**< 0x26 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_paddr_write",     g_aSvcX86PAddrWrite),                                     /**< 0x27 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_smu_send_msg",        g_aSvcSmuSendMsg),                                        /**< 0x28 */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x29 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_smu_region_reserve",  g_aSvcSmuRsv),                                            /**< 0x2a */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x2b Unknown, MCM communication related. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x2c Unknown, clears flag in global PSP memory. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x2d Unknown, interfaces the flash. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x2e Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x2f Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x30 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_call_slave_psp",      g_aSvcCallSlvPsp),                                        /**< 0x31 Calls a slave PSP on a different CCX. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x32 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_0x33",                g_aSvc0x33),                                              /**< 0x33 Unknown. */
    SVC_TRACE_SVC_INIT_NO_ARGS(  "psp_svc_platform_reset"),                                                               /**< 0x34 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_0x35",                g_aSvc0x35),                                              /**< 0x35 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x36 Unknown, interfaces the CCP. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_mem_inv_clean",       g_aSvcMemInvClean),                                       /**< 0x37 invalidate/clean memory region. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_ccp_req",             g_aSvcCcpReq),                                            /**< 0x38 */
    SVC_TRACE_SVC_INIT_NO_ARGS(  "psp_svc_trng_fill_buf"),                                                                /**< 0x39 */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_asid_set_min_non_es", g_aSvcAsidSetMinNonEs),                                   /**< 0x3a */
    SVC_TRACE_SVC_INIT_NO_ARGS(  "psp_svc_asid_get_min_non_es"),                                                          /**< 0x3a */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x3b Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x3c Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x3d Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x3e Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x3f Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x40 Unknown. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_0x41",                g_aSvc0x41),                                              /**< 0x41 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x42 Unknown, accesses the CCP. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x43 Unknown, accesses the flash and PSP filesystem. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x44 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x45 Not provided. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x46 Not provided. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x47 Unknown, CCP zlib decompression. */
    SVC_TRACE_SVC_INIT_DEF(      "psp_svc_x86_query_smm",       g_aSvcX86QuerySmm),                                       /**< 0x48 */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x49 Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x4a Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x4b Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x4c Unknown. */
    SVC_TRACE_SVC_INIT_UNKNOWN,                                                                                           /**< 0x4d Unknown. */
};

#endif

