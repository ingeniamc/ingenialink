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

#include "osal/err.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_timer_t *osal_timer_create()
{
	osal_timer_t *timer;

	timer = malloc(sizeof(*timer));
	if (!timer)
		return NULL;

	timer->hnd = CreateWaitableTimer(NULL, FALSE, NULL);
	if (!timer->hnd)
		goto cleanup_timer;

	return timer;

cleanup_timer:
	free(timer);

	return NULL;
}

void osal_timer_destroy(osal_timer_t *timer)
{
	CancelWaitableTimer(timer->hnd);
	CloseHandle(timer->hnd);
	free(timer);
}

int osal_timer_set(osal_timer_t *timer, osal_time_t period)
{
	LARGE_INTEGER period_;

	period_.QuadPart = -(LONGLONG)period / STEP_SIZE;

	if (!SetWaitableTimer(timer->hnd, &period_,
			      (LONG)period / OSAL_TIMER_NANOSPERMSEC,
			      NULL, NULL, FALSE))
		return OSAL_EFAIL;

	return 0;
}

int osal_timer_wait(osal_timer_t *timer)
{
	if (WaitForSingleObject(timer->hnd, INFINITE) != WAIT_OBJECT_0) {
		if (GetLastError() == ERROR_TIMEOUT)
			return OSAL_ETIMEDOUT;

		return OSAL_EFAIL;
	}

	return 0;
}

