#ifndef PUBLIC_INGENIALINK_ERR_H_
#define PUBLIC_INGENIALINK_ERR_H_

#include "common.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/err.h
 * @brief Error reporting.
 * @defgroup IL_ERR Error reporting
 * @ingroup IL
 * @{
 */

/*
 * Library error codes.
 */

/** General failure. */
#define IL_EFAIL	-1
/** Invalid values. */
#define IL_EINVAL       -2
/** Operation timed out. */
#define IL_ETIMEDOUT    -3
/** Not enough memory. */
#define IL_ENOMEM	-4
/** Already initialized. */
#define IL_EALREADY	-5
/** Device disconnected. */
#define IL_EDISCONN	-6
/** Access error. */
#define IL_EACCESS	-7
/** State error. */
#define IL_ESTATE	-8
/** I/O error. */
#define IL_EIO		-9
/** Not supported. */
#define IL_ENOTSUP	-10
/** Wrong register obtained. */
#define IL_EWRONGREG -11
/** Wrong CRC. */
#define IL_EWRONGCRC -12
/** NACK. */
#define IL_ENACK -13
/** Register not found in dictionary. */
#define IL_REGNOTFOUND -14

/**
 * Obtain library last error details.
 *
 * @note
 *     If host target supports thread local storage (TLS) the last error
 *     description is kept on a per-thread basis.
 *
 * @return
 *      Last error details.
 */
IL_EXPORT const char *ilerr_last(void);
IL_EXPORT int *ilerr_ipb_last(void);
IL_EXPORT void set_log_level(int level);

/** @} */

IL_END_DECL

#endif
