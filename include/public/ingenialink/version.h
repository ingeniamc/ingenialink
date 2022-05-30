#ifndef PUBLIC_INGENIALINK_VERSION_H_
#define PUBLIC_INGENIALINK_VERSION_H_

#include "common.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/version.h
 * @brief Version information.
 * @defgroup IL_VERSION Version information
 * @ingroup IL
 * @{
 */

/**
 * Obtain library version.
 *
 * @return
 *	  Library version (e.g. "1.0.0")
 */
IL_EXPORT const char *il_version(void);
/** @} */

IL_EXPORT void free_(void *pointer);

IL_END_DECL

#endif
