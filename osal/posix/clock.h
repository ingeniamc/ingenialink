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

#ifndef OSAL_POSIX_CLOCK_H_
#define OSAL_POSIX_CLOCK_H_

#include "osal/clock.h"

#if defined(__MACH__) && defined(__APPLE__)
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#elif defined(__linux__)
#  include <time.h>
#endif

 /** Performance counter (POSIX) */
struct osal_clock_perf {
#if defined(__MACH__) && defined(__APPLE__)
	/** Timebase info. */
	mach_timebase_info_data_t tb;
	/** Reference time. */
	uint64_t started;
#elif defined(__linux__)
	/** Reference time. */
	struct timespec started;
#endif
};

#endif
