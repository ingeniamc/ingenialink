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

#ifndef OSAL_CLOCK_H_
#define OSAL_CLOCK_H_

/** Nanoseconds per second. */
#define OSAL_CLOCK_NANOSPERSEC     1000000000LL

/** Nanoseconds per millisecond. */
#define OSAL_CLOCK_NANOSPERMSEC    1000000L

/** Nanoseconds per microsecond. */
#define OSAL_CLOCK_NANOSPERUSEC    1000

/** Timespec. */
typedef struct {
	/** Seconds. */
	long s;
	/** Nanoseconds. */
	long ns;
} osal_timespec_t;

/** Performance counter. */
typedef struct osal_clock_perf osal_clock_perf_t;

/**
 * Create a performance counter.
 *
 * @return
 *	Performance counter instance (NULL if it could not be created).
 */
osal_clock_perf_t *osal_clock_perf_create(void);

/**
 * Destroy a performance counter.
 *
 * @param [in] perf
 *	Performance counter instance.
 */
void osal_clock_perf_destroy(osal_clock_perf_t *perf);

/**
 * Reset a performance counter start time reference.
 *
 * @param [in] perf
 *	Performance counter instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
int osal_clock_perf_reset(osal_clock_perf_t *perf);

/**
 * Obtain performance counter using the highest available resolution clock on
 * the system.
 *
 * @note
 *      The counter is monotonic.
 */
int osal_clock_perf_get(osal_clock_perf_t *perf, osal_timespec_t *ts);

/**
 * Sleep (ms).
 *
 * @param [in] ms
 *	Number of milliseconds to sleep.
 */
void osal_clock_sleep_ms(int ms);

#endif
