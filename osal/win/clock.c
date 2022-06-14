#include "clock.h"

/*
 * NOTE: QPC calls never fail on Windows XP or later!
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
	(void)QueryPerformanceFrequency(&perf->freq);
	(void)QueryPerformanceCounter(&perf->start);

	return 0;
}

int osal_clock_perf_get(osal_clock_perf_t *perf, osal_timespec_t *ts)
{
	LARGE_INTEGER cnt;

	(void)QueryPerformanceCounter(&cnt);

	/* convert to microseconds relative to start */
	cnt.QuadPart -= perf->start.QuadPart;
	cnt.QuadPart *= 1000000;
	cnt.QuadPart /= perf->freq.QuadPart;

	/* convert to timespec */
	ts->ns = (long)((cnt.QuadPart % 1000000) * 1000);
	ts->s = (long)(cnt.QuadPart / 1000000);

	return 0;
}

int osal_clock_gettime(osal_timespec_t *ts)
{
	LARGE_INTEGER freq, cnt;

	(void)QueryPerformanceFrequency(&freq);
	(void)QueryPerformanceCounter(&cnt);

	/* convert to microseconds */
	cnt.QuadPart *= 1000000;
	cnt.QuadPart /= freq.QuadPart;

	/* convert to timespec */
	ts->ns = (long)((cnt.QuadPart % 1000000) * 1000);
	ts->s = (long)(cnt.QuadPart / 1000000);

	return 0;
}

void osal_clock_sleep_ms(int ms)
{
	Sleep(ms);
}

