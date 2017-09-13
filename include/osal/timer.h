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

#ifndef OSAL_TIMER_H_
#define OSAL_TIMER_H_

/** Nanoseconds per second. */
#define OSAL_TIMER_NANOSPERSEC		1000000000LL

/** Nanoseconds per millisecond. */
#define OSAL_TIMER_NANOSPERMSEC		1000000L

/** Nanoseconds per microsecond. */
#define OSAL_TIMER_NANOSPERUSEC		1000

/** Time value (nanoseconds base). */
typedef long long int osal_time_t;

/** Timer instance. */
typedef struct osal_timer osal_timer_t;

/**
 * Create a timer.
 *
 * @return
 *	Timer instance (NULL if it could not be created).
 */
osal_timer_t *osal_timer_create(void);

/**
 * Destroy a timer.
 *
 * @param [in] timer
 *	Timer instance.
 */
void osal_timer_destroy(osal_timer_t *timer);

/**
 * Set (arm) a timer.
 *
 * @param [in] timer
 *	Timer instance.
 * @param [in] period
 *	Timer period (ns).
 *
 * @return
 *	0 on success, error code otherwise.
 */
int osal_timer_set(osal_timer_t *timer, osal_time_t period);

/**
 * Wait until timer expires.
 *
 * @param [in] timer
 *	Timer instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
int osal_timer_wait(osal_timer_t *timer);

#endif
