/*
 * MIT License
 *
 * Copyright (c) 2017 Ingenia-CAT S.L.
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

#ifndef INGENIALINK_UTILS_H_
#define INGENIALINK_UTILS_H_

#include "public/ingenialink/common.h"

/** Obtain the minimum of a, b. */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** Swap 16-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_16(x) \
	((((uint16_t)(x) & 0xFF00U) >> 8) | \
	 (((uint16_t)(x) & 0x00FFU) << 8))
#else
#define __swap_16(x) (x)
#endif

/** Swap 32-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_32(x) \
	((((uint32_t)(x) & 0xFF000000U) >> 24) | \
	 (((uint32_t)(x) & 0x00FF0000U) >>  8) | \
	 (((uint32_t)(x) & 0x0000FF00U) <<  8) | \
	 (((uint32_t)(x) & 0x000000FFU) << 24))
#else
#define __swap_32(x) (x)
#endif

/** Swap 64-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_64(x) \
	((((uint64_t)(x) & 0xFF00000000000000U) >> 56) | \
	 (((uint64_t)(x) & 0x00FF000000000000U) >> 40) | \
	 (((uint64_t)(x) & 0x0000FF0000000000U) >> 24) | \
	 (((uint64_t)(x) & 0x000000FF00000000U) >>  8) | \
	 (((uint64_t)(x) & 0x00000000FF000000U) <<  8) | \
	 (((uint64_t)(x) & 0x0000000000FF0000U) << 24) | \
	 (((uint64_t)(x) & 0x000000000000FF00U) << 40) | \
	 (((uint64_t)(x) & 0x00000000000000FFU) << 56))
#else
#define __swap_64(x) (x)
#endif

#endif
