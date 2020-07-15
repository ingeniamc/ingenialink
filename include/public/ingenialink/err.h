/*
 * MIT License
 *
 * Copyright (c) 2017-2018 Ingenia-CAT S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
/** Wrong Register. */
#define IL_EWRONGREG	-11

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

/** @} */

IL_END_DECL

#endif
