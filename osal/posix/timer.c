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

#include "timer.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__linux__)
#  include <sys/timerfd.h>
#  include <unistd.h>
#endif

#include "osal/err.h"

/*
 * References:
 *      TN2169 "High Precision Timers in iOS / OS X", Apple Computer Inc.
 *      https://developer.apple.com/library/content/technotes/tn2169/_index.html
 */

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_timer_t *osal_timer_create()
{
	osal_timer_t *timer;

	timer = malloc(sizeof(*timer));
	if (!timer)
		return NULL;

#if defined(__MACH__) && defined(__APPLE__)
	mach_timebase_info(&timer->tb);

	return timer;
#elif defined(__linux__)
	timer->t = timerfd_create(CLOCK_MONOTONIC, 0);
	if (timer->t < 0)
		goto cleanup_timer;

	return timer;

cleanup_timer:
	free(timer);

	return NULL;
#endif
}

void osal_timer_destroy(osal_timer_t *timer)
{
#if defined(__linux__)
	close(timer->t);
#endif
	free(timer);
}

int osal_timer_set(osal_timer_t *timer, osal_time_t period)
{
#if defined(__MACH__) && defined(__APPLE__)
	timer->period = (period * timer->tb.denom) / timer->tb.numer;
	timer->target = mach_absolute_time() + timer->period;
#elif defined(__linux__)
	struct timespec now;
	struct itimerspec its;

	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
		return OSAL_EFAIL;

	its.it_value.tv_nsec = now.tv_nsec;
	its.it_value.tv_sec = now.tv_sec;

	its.it_interval.tv_sec = period / OSAL_TIMER_NANOSPERSEC;
	its.it_interval.tv_nsec = period % OSAL_TIMER_NANOSPERSEC;

	if (timerfd_settime(timer->t, TFD_TIMER_ABSTIME, &its, NULL) < 0)
		return OSAL_EFAIL;
#endif

	return 0;
}

int osal_timer_wait(osal_timer_t *timer)
{
#if defined(__MACH__) && defined(__APPLE__)
	mach_wait_until(timer->target);
	timer->target += timer->period;
#elif defined(__linux__)
	uint64_t expirations;

	if (read(timer->t, &expirations, sizeof(expirations)) < 0)
		return OSAL_EFAIL;
#endif

	return 0;
}
