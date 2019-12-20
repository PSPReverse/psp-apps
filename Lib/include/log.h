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
#ifndef __log_h
#define __log_h

#include <types.h>
#include <stdarg.h>

#include <cdefs.h>
#include <tm.h>

/** Log flush callback. */
typedef void FNLOGGERFLUSH(void *pvUser, uint8_t *pbBuf, size_t cbBuf);
/** Pointer to a flush callback. */
typedef FNLOGGERFLUSH *PFNLOGGERFLUSH;

/** A new line was started, the log ID and timestamp might be prepended. */
#define LOG_LOGGER_FLAGS_NEW_LINE BIT(10)

/**
 * Logging instance.
 */
typedef struct LOGGER
{
    /** Scratch buffer used for formatting. */
    char           achScratch[1024];
    /** offset into the scratch buffer. */
    unsigned       offScratch;
    /** Logger identifier given during creation. */
    const char    *pszLogId;
    /** Pointer to the time manager to get timestamps. */
    PTM            pTm;
    /** Pointer to log flush callback. */
    PFNLOGGERFLUSH pfnFlush;
    /** Opaque user data for the flush callback. */
    void          *pvUser;
    /** Internal flags for the logger. */
    uint32_t       fFlags;
} LOGGER;
/** Pointer to a logging instance. */
typedef LOGGER *PLOGGER;

/** Debug logging workers, taking LOG_ENABLED into account. */
#ifdef LOG_ENABLED
# define Log(a_pszFmt, ...) LOGLogger(NULL, a_pszFmt, ##__VA_ARGS__)
#else
# define Log(a_pszFmt, ...) do {} while (0)
#endif

/** Release logger worker, always logs no matter whether LOG_ENABLED is set. */
#define LogRel(a_pszFmt, ...) LOGLogger(NULL, a_pszFmt, ##__VA_ARGS__)

/** The timestamp output format will be HH:MM:SS.mmm instead of just
 * millisecond. */
#define LOG_LOGGER_INIT_FLAGS_TS_FMT_HHMMSS BIT(0)

/**
 * Initialises a new logger instance.
 *
 * @returns status code.
 * @param   pLogger     Pointer to the logger instance data to initialise.
 * @param   pfnFlush    Logging flush callback.
 * @param   pvUser      Opaque user data for the flush callback.
 * @param   pszLogId    Optional logging ID which is prepended before the actual message.
 * @param   pTm         Optional timekeeping manager to get timestamps in the log.
 * @param   fFlags      Flags for the logger, see LOG_LOGGER_INIT_FLAGS_* defines.
 */
int LOGLoggerInit(PLOGGER pLogger, PFNLOGGERFLUSH pfnFlush, void *pvUser,
                  const char *pszLogId, PTM pTm, uint32_t fFlags);

/**
 * Returns the default logging instance.
 *
 * @returns default logging instance or NULL if none was set.
 */
PLOGGER LOGLoggerGetDefaultInstance(void);

/**
 * Sets a new default logging instance and returns the  old one.
 *
 * @returns default logging instance or NULL if none was set.
 * @param   pLogger     The new default logging instance.
 */
PLOGGER LOGLoggerSetDefaultInstance(PLOGGER pLogger);

/**
 * Main logging function.
 *
 * @returns nothing.
 * @param   pLogger    The logger instance to use, if NULL the default instance is used.
 * @param   pszFmt     Format string.
 * @param   ...        Format arguments.
 */
void LOGLogger(PLOGGER pLogger, const char *pszFmt, ...);

void LOGLoggerV(PLOGGER pLogger, const char *pszFmt, va_list hArgs);

#endif /* __log_h */
