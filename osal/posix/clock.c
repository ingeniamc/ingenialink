#include "clock.h"

#include <stdlib.h>
#include <unistd.h>

#include "osal/err.h"

/*
 * References:
 *      TN2169 "High Precision Timers in iOS / OS X", Apple Computer Inc.
 *      https://developer.apple.com/library/content/technotes/tn2169/_index.html
 */

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_clock_perf_t *osal_clock_perf_create()
{
	osal_clock_perf_t *perf;

	perf = malloc(sizeof(*perf));
	if (!perf)
		return NULL;

	osal_clock_perf_reset(perf);

	return perf;
}

void osal_clock_perf_destroy(osal_clock_perf_t *perf)
{
	free(perf);
}

int osal_clock_perf_reset(osal_clock_perf_t *perf)
{
#if defined(__MACH__) && defined(__APPLE__)
	mach_timebase_info(&perf->tb);
	perf->started = mach_absolute_time();
#elif defined(__linux__)
	if (clock_gettime(CLOCK_MONOTONIC, &perf->started) < 0)
		return OSAL_EFAIL;
#endif

	return 0;
}

int osal_clock_perf_get(osal_clock_perf_t *perf, osal_timespec_t *ts)
{
#if defined(__MACH__) && defined(__APPLE__)
	uint64_t time;

	time = ((mach_absolute_time() - perf->started) *
		perf->tb.numer) / perf->tb.denom;

	ts->s = time / OSAL_CLOCK_NANOSPERSEC;
	ts->ns = time % OSAL_CLOCK_NANOSPERSEC;
#else
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
		return OSAL_EFAIL;

	if ((now.tv_nsec - perf->started.tv_nsec) < 0) {
		ts->s = now.tv_sec - perf->started.tv_sec - 1;
		ts->ns = now.tv_nsec - perf->started.tv_nsec +
			 OSAL_CLOCK_NANOSPERSEC;
	} else {
		ts->s = now.tv_sec - perf->started.tv_sec;
		ts->ns = now.tv_nsec - perf->started.tv_nsec;
	}
#endif

	return 0;
}

int osal_clock_gettime(osal_timespec_t *ts)
{
#if defined(__MACH__) && defined(__APPLE__)
	mach_timebase_info_data_t tb;
	uint64_t time;

	mach_timebase_info(&tb);
	time = (mach_absolute_time() * tb.numer) / tb.denom;

	ts->s = time / OSAL_CLOCK_NANOSPERSEC;
	ts->ns = time % OSAL_CLOCK_NANOSPERSEC;
#else
	struct timespec time;

	if (clock_gettime(CLOCK_MONOTONIC, &time) < 0)
		return OSAL_EFAIL;

	ts->s = time.tv_sec;
	ts->ns = time.tv_nsec;
#endif

	return 0;
}

void osal_clock_sleep_ms(int ms)
{
	usleep(ms * 1000);
}

