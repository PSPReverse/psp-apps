/** @file
 * Logging API and macros - inspired by the VirtualBox IPRT Logging API which is really neat.
 */

/*
 * Copyright (C) 2013 Alexander Eichner <aeichner@aeichner.de>
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
#include <stddef.h>
#include <stdarg.h>

#include <err.h>
#include <log.h>
#include <cdefs.h>
#include <string.h>

/** The default logging instance, one of the few global variables in the library
 * we can't avoid without making everything else more difficult. */
static PLOGGER g_pLoggerDef = NULL;

/**
 * Flushes the given logging instance.
 *
 * @returns nothing.
 * @param   pLogger    The logger to flush.
 */
static void logLoggerFlush(PLOGGER pLogger)
{
    if (pLogger->offScratch > 0)
    {
        pLogger->pfnFlush(pLogger->pvUser, (uint8_t *)&pLogger->achScratch[0], pLogger->offScratch);
        pLogger->offScratch = 0;
    }
}

/**
 * Appends a single character to the given logger instance.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   ch         The character to append.
 */
static void logLoggerAppendChar(PLOGGER pLogger, const char ch)
{
    if (pLogger->offScratch == sizeof(pLogger->achScratch))
        logLoggerFlush(pLogger);

    pLogger->achScratch[pLogger->offScratch] = ch;
    pLogger->offScratch++;
}

/**
 * Converts a given unsigned 32bit integer into a string and appends it to the scratch buffer.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   u32        The value to log.
 * @param   cDigits    Minimum number of digits to log, if the number has fewer
 *                     the gap is prepended with 0.
 */
static void logLoggerAppendU32(PLOGGER pLogger, uint32_t u32, uint32_t cDigits)
{
    char achDigits[] = "0123456789";
    char aszBuf[32];
    unsigned offBuf = 0;

    /** @todo: Optimize. */

    while (u32)
    {
        uint8_t u8Val = u32 % 10;
        u32 /= 10;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    /* Prepend 0. */
    if (offBuf < cDigits)
    {
        while (cDigits - offBuf > 0)
        {
            logLoggerAppendChar(pLogger, '0');
            cDigits--;
        }
    }

    while (offBuf-- > 0)
        logLoggerAppendChar(pLogger, aszBuf[offBuf]);
}

#if 0
/**
 * Converts a given unsigned 64bit integer into a string and appends it to the scratch buffer.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   u64        The value to log.
 * @param   cDigits    Minimum number of digits to log, if the number has fewer
 *                     the gap is prepended with 0.
 */
static void logLoggerAppendU64(PLOGGER pLogger, uint64_t u64, uint32_t cDigits)
{
    char achDigits[] = "0123456789";
    char aszBuf[32];
    unsigned offBuf = 0;

    /** @todo: Optimize. */

    while (u64)
    {
        uint8_t u8Val = u64 % 10;
        u64 /= 10;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    /* Prepend 0. */
    if (offBuf < cDigits)
    {
        while (cDigits - offBuf > 0)
        {
            logLoggerAppendChar(pLogger, '0');
            cDigits--;
        }
    }

    while (offBuf-- > 0)
        logLoggerAppendChar(pLogger, aszBuf[offBuf]);
}
#endif

/**
 * Converts a given unsigned 32bit integer into a string as hex and appends it to the scratch buffer.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   u32        The value to log.
 * @param   cDigits    Minimum number of digits to log, if the number has fewer
 *                     the gap is prepended with 0.
 */
static void logLoggerAppendHexU32(PLOGGER pLogger, uint32_t u32, uint32_t cDigits)
{
    char achDigits[] = "0123456789abcdef";
    char aszBuf[10];
    unsigned offBuf = 0;

    /** @todo: Optimize. */

    while (u32)
    {
        uint8_t u8Val = u32 & 0xf;
        u32 >>= 4;

        aszBuf[offBuf++] = achDigits[u8Val];
    }

    /* Prepend 0. */
    if (offBuf < cDigits)
    {
        while (cDigits - offBuf > 0)
        {
            logLoggerAppendChar(pLogger, '0');
            cDigits--;
        }
    }

    while (offBuf-- > 0)
        logLoggerAppendChar(pLogger, aszBuf[offBuf]);
}

/**
 * Converts a given unsigned 64bit integer into a string as hex and appends it to the scratch buffer.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   u64        The value to log.
 * @param   cDigits    Minimum number of digits to log, if the number has fewer
 *                     the gap is prepended with 0.
 */
static void logLoggerAppendHexU64(PLOGGER pLogger, uint64_t u64, uint32_t cDigits)
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

    /* Prepend 0. */
    if (offBuf < cDigits)
    {
        while (cDigits - offBuf > 0)
        {
            logLoggerAppendChar(pLogger, '0');
            cDigits--;
        }
    }

    while (offBuf-- > 0)
        logLoggerAppendChar(pLogger, aszBuf[offBuf]);
}

/**
 * Converts a given signed 32bit integer into a string and appends it to the scratch buffer.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance.
 * @param   i32        The value to log.
 * @param   cDigits    Minimum number of digits to log, if the number has fewer
 *                     the gap is prepended with 0.
 */
static void logLoggerAppendS32(PLOGGER pLogger, int32_t i32, uint32_t cDigits)
{
    /* Add sign? */
    if (i32 < 0)
    {
        logLoggerAppendChar(pLogger, '-');
        i32 = ABS(i32);
    }

    /* Treat as unsigned from here on. */
    logLoggerAppendU32(pLogger, (uint32_t)i32, cDigits);
}

/**
 * Appends a given string to the logger instance.
 *
 * @returns nothing.
 * @param   pLogger The logger instance.
 * @param   psz     The string to append.
 */
static void logLoggerAppendString(PLOGGER pLogger, const char *psz)
{
    /** @todo: Optimize */
    if (!psz)
    {
        logLoggerAppendString(pLogger, "<null>");
        return;
    }

    while (*psz)
        logLoggerAppendChar(pLogger, *psz++);
}

/**
 * Adds the log id and a timestamp to the logger.
 *
 * @returns nothing.
 * @param   pLogger    Logger instance to use.
 */
static void logLoggerAddLogIdAndTimestamp(PLOGGER pLogger)
{
    if (pLogger->pTm)
    {
        uint32_t cMs = TMGetMillies(pLogger->pTm);
        if (pLogger->fFlags & LOG_LOGGER_INIT_FLAGS_TS_FMT_HHMMSS)
        {
            uint16_t uHour, uMin, uSec, uMs;

            uSec = MSEC_TO_SEC(cMs);
            uMs  = MSEC_TO_SEC_REMAINDER(cMs);

            uMin = SEC_TO_MIN(uSec);
            uHour = MIN_TO_HOUR(uMin);

            uMin = MIN_TO_HOUR_REMAINDER(uMin);
            uSec = SEC_TO_MIN_REMAINDER(uSec);

            logLoggerAppendU32(pLogger, uHour, 2);
            logLoggerAppendChar(pLogger, ':');
            logLoggerAppendU32(pLogger, uMin, 2);
            logLoggerAppendChar(pLogger, ':');
            logLoggerAppendU32(pLogger, uSec, 2);
            logLoggerAppendChar(pLogger, '.');
            logLoggerAppendU32(pLogger, uMs, 3);
        }
        else
            logLoggerAppendU32(pLogger, cMs, 6);
        logLoggerAppendChar(pLogger, ' ');
    }

    if (pLogger->pszLogId)
    {
        logLoggerAppendString(pLogger, pLogger->pszLogId);
        logLoggerAppendChar(pLogger, ' ');
    }
}

int LOGLoggerInit(PLOGGER pLogger, PFNLOGGERFLUSH pfnFlush, void *pvUser,
                  const char *pszLogId, PTM pTm, uint32_t fFlags)
{
    int rc = INF_SUCCESS;

    pLogger->offScratch = 0;
    pLogger->pszLogId   = pszLogId;
    pLogger->pTm        = pTm;
    pLogger->pfnFlush   = pfnFlush;
    pLogger->pvUser     = pvUser;
    pLogger->fFlags     = LOG_LOGGER_FLAGS_NEW_LINE | fFlags;

    return rc;
}

PLOGGER LOGLoggerGetDefaultInstance(void)
{
    return g_pLoggerDef;
}

PLOGGER LOGLoggerSetDefaultInstance(PLOGGER pLogger)
{
    PLOGGER pLoggerOld = g_pLoggerDef;

    g_pLoggerDef = pLogger;

    return pLoggerOld;
}

void LOGLoggerV(PLOGGER pLogger, const char *pszFmt, va_list hArgs)
{
    /* Try default logger if none is given. */
    if (!pLogger)
    {
        if (!g_pLoggerDef)
            return;

        pLogger = g_pLoggerDef;
    }

    while (*pszFmt)
    {
        char ch = *pszFmt++;

        if (pLogger->fFlags & LOG_LOGGER_FLAGS_NEW_LINE)
        {
            logLoggerAddLogIdAndTimestamp(pLogger);
            pLogger->fFlags &= ~LOG_LOGGER_FLAGS_NEW_LINE;
        }

        switch (ch)
        {
            case '\n':
            {
                logLoggerAppendChar(pLogger, ch);
                pLogger->fFlags |= LOG_LOGGER_FLAGS_NEW_LINE;
                break;
            }
            case '%':
            {
                /* Format specifier. */
                char chFmt = *pszFmt;
                pszFmt++;

                if (chFmt == '#')
                {
                    logLoggerAppendString(pLogger, "0x");
                    chFmt = *pszFmt++;
                }

                switch (chFmt)
                {
                    case '%':
                    {
                        logLoggerAppendChar(pLogger, '%');
                        break;
                    }
                    case 'u':
                    {
                        uint32_t u32 = va_arg(hArgs, uint32_t);
                        logLoggerAppendU32(pLogger, u32, 1);
                        break;
                    }
#if 0
                    case 'U':
                    {
                        uint64_t u64 = va_arg(hArgs, uint64_t);
                        logLoggerAppendU64(pLogger, u64, 1);
                        break;
                    }
#endif
                    case 'd':
                    {
                        int32_t i32 = va_arg(hArgs, int32_t);
                        logLoggerAppendS32(pLogger, i32, 1);
                        break;
                    }
                    case 's':
                    {
                        const char *psz = va_arg(hArgs, const char *);
                        logLoggerAppendString(pLogger, psz);
                        break;
                    }
                    case 'x':
                    {
                        uint32_t u32 = va_arg(hArgs, uint32_t);
                        logLoggerAppendHexU32(pLogger, u32, 1);
                        break;
                    }
                    case 'X':
                    {
                        uint64_t u64 = va_arg(hArgs, uint64_t);
                        logLoggerAppendHexU64(pLogger, u64, 1);
                        break;
                    }
                    case 'p': /** @todo: Works only on 32bit... */
                    {
                        void *pv = va_arg(hArgs, void *);
                        logLoggerAppendString(pLogger, "0x");
                        if (sizeof(void *) == 4)
                            logLoggerAppendHexU32(pLogger, (uint32_t)pv,  8);
#ifdef __AMD64__
                        else if (sizeof(void *) == 8)
                            logLoggerAppendHexU64(pLogger, (uint64_t)pv, 16);
#endif
                        else
                            logLoggerAppendString(pLogger, "<Unrecognised pointer width>");
                    }
                    default:
                        /** @todo: Ignore or assert? */
                        ;
                }
                break;
            }
            default:
                logLoggerAppendChar(pLogger, ch);
        }
    }

    logLoggerFlush(pLogger);
}

void LOGLogger(PLOGGER pLogger, const char *pszFmt, ...)
{
    va_list hArgs;
    va_start(hArgs, pszFmt);

    LOGLoggerV(pLogger, pszFmt, hArgs);

    va_end(hArgs);
}

