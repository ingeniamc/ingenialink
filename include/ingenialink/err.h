#ifndef INGENIALINK_ERR_H_
#define INGENIALINK_ERR_H_

#include "public/ingenialink/err.h"

/**
 * Set last error message.
 *
 * @param [in] fmt
 *      Format string.
 * @param [in] ...
 *      Arguments for format specification.
 */
void ilerr__set(const char *fmt, ...);

/**
 * Set last error message with libsercomm error details and return a matching
 * libingenialink error code.
 *
 * @param [in] code
 *      libsercomm error code.
 *
 * @return
 *      Matching libingenialink error code (IL_EFAIL if none matches).
 */
int ilerr__ser(int32_t code);

int ilerr__eth(int32_t code);

int ilerr__ecat(int32_t code);

#endif
