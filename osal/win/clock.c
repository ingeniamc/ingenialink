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

void osal_clock_sleep_ms(int ms)
{
	Sleep(ms);
}

