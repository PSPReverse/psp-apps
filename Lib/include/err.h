#ifndef __include_err_h
#define __include_err_h

#define PSPSTATUS_SUCCESS (0)

/** Status code success indicator. */
#define INF_SUCCESS (0)

/** Operation still pending, try again later. */
#define INF_TRY_AGAIN         (1)
/** Invalid parameter passed. */
#define ERR_INVALID_PARAMETER (-1)
/** Buffer overflow. */
#define ERR_BUFFER_OVERFLOW   (-2)
/** Not implemented at the moment. */
#define ERR_NOT_IMPLEMENTED   (-3)
/** Invalid state encountered. */
#define ERR_INVALID_STATE     (-4)

/**
 * USS specific error codes.
 */
/** The remaining data of the telegram is too short to get the requested value. */
#define ERR_USS_TLG_TOO_SHORT                            (-100)
/** The telegram parser is not in the correct state to execute the request. */
#define ERR_USS_TLG_PARSER_INVALID_STATE                 (-101)
/** The telegram still has unparsed data left. */
#define WRN_USS_TLG_PARSER_DATA_LEFT                     (100)

/**
 * USS transport driver specific error codes.
 */
/** No time keeping manager was given during creation. The periodic telegram feature
 * is not available. */
#define ERR_USS_TRANSP_NO_TIMER                          (-200)
/** The periodic callback was already set. */
#define ERR_USS_TRANSP_PERIODIC_TLG_CALLBACK_ALREADY_SET (-201)

/**
 * TM (Timekeeping manager) specific error codes.
 */
/** There are no free slots left in the time keeping manager. */
#define ERR_TM_OUT_OF_SLOTS                              (-300)

/**
 * NORD frequency inverter error codes.
 */
/** There is a mismatch between the size of the telegrams. */
#define ERR_NORD_MISMATCH_TELEGRAM_SIZE                  (-400)
/** The frequency inverter is not responding in a timely fashion. */
#define ERR_NORD_NOT_RESPONDING                          (-401)

/**
 * Flow controller error codes.
 */
/** There are not enough datapoints collected to get a trend. */
#define ERR_FLOWCTL_NOT_ENOUGH_DATAPOINTS_FOR_TREND      (-500)

#endif
