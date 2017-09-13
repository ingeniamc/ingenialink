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

#include "poller.h"

#include <stdlib.h>
#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

int poller_thread(void *args)
{
	il_poller_t *poller = args;
	osal_timespec_t curr;

	while (!poller->stop) {
		double t, d;

		/* wait until next period, obtain current time */
		osal_timer_wait(poller->timer);
		osal_clock_perf_get(poller->perf, &curr);

		/* poll */
		il_axis_read(poller->axis, poller->reg, &d);
		t = (double)curr.s + (double)curr.ns / 1000000000.;

		/* push to the buffer */
		osal_mutex_lock(poller->buf_lock);

		if (poller->buf_cnt < poller->buf_sz) {
			poller->t_buf[poller->buf_curr][poller->buf_cnt] = t;
			poller->d_buf[poller->buf_curr][poller->buf_cnt] = d;

			poller->buf_cnt++;
		} else {
			poller->lost = 1;
		}

		osal_mutex_unlock(poller->buf_lock);
	}

	return 0;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_poller_t *il_poller_create(il_axis_t *axis, const il_reg_t *reg, int period,
			      size_t sz)
{
	il_poller_t *poller;

	poller = malloc(sizeof(*poller));
	if (!poller) {
		ilerr__set("Poller allocation failed");
		return NULL;
	}

	poller->timer = osal_timer_create();
	if (!poller->timer) {
		ilerr__set("Poller timer allocation failed");
		goto cleanup_poller;
	}

	poller->perf = osal_clock_perf_create();
	if (!poller->perf) {
		ilerr__set("Poller performance counter allocation failed");
		goto cleanup_timer;
	}

	/* allocate buffers and its associated resources */
	poller->t_buf[0] = malloc(sizeof(poller->t_buf[0]) * sz);
	if (!poller->t_buf[0]) {
		ilerr__set("Poller buffer allocation failed");
		goto cleanup_perf;
	}

	poller->t_buf[1] = malloc(sizeof(poller->t_buf[1]) * sz);
	if (!poller->t_buf[1]) {
		ilerr__set("Poller buffer allocation failed");
		goto cleanup_t_buf0;
	}

	poller->d_buf[0] = malloc(sizeof(poller->d_buf[0]) * sz);
	if (!poller->t_buf[0]) {
		ilerr__set("Poller buffer allocation failed");
		goto cleanup_t_buf1;
	}

	poller->d_buf[1] = malloc(sizeof(poller->d_buf[1]) * sz);
	if (!poller->d_buf[1]) {
		ilerr__set("Poller buffer allocation failed");
		goto cleanup_d_buf0;
	}

	poller->buf_curr = 0;
	poller->buf_cnt = 0;
	poller->buf_sz = sz;

	poller->buf_lock = osal_mutex_create();
	if (!poller->buf_lock) {
		ilerr__set("Buffer lock allocation failed");
		goto cleanup_d_buf1;
	}

	poller->axis = axis;
	poller->reg = reg;
	poller->period = period;
	poller->stop = 0;
	poller->running = 0;

	return poller;

cleanup_d_buf1:
	free(poller->d_buf[1]);

cleanup_d_buf0:
	free(poller->d_buf[0]);

cleanup_t_buf1:
	free(poller->t_buf[1]);

cleanup_t_buf0:
	free(poller->t_buf[0]);

cleanup_perf:
	osal_clock_perf_destroy(poller->perf);

cleanup_timer:
	osal_timer_destroy(poller->timer);

cleanup_poller:
	free(poller);

	return NULL;
}

void il_poller_destroy(il_poller_t *poller)
{
	if (poller->running)
		il_poller_stop(poller);

	osal_mutex_destroy(poller->buf_lock);

	free(poller->d_buf[1]);
	free(poller->d_buf[0]);
	free(poller->t_buf[1]);
	free(poller->t_buf[0]);

	osal_clock_perf_destroy(poller->perf);
	osal_timer_destroy(poller->timer);

	free(poller);
}

int il_poller_start(il_poller_t *poller)
{
	if (!poller) {
		ilerr__set("Invalid poller (NULL)");
		return IL_EFAULT;
	}

	if (poller->running) {
		ilerr__set("Poller already running");
		return IL_EALREADY;
	}

	poller->lost = 0;

	/* activate timer, reset performance counter */
	if (osal_timer_set(poller->timer,
			   poller->period * OSAL_TIMER_NANOSPERMSEC) < 0) {
		ilerr__set("Timer activation failed");
		return IL_EFAIL;
	}

	if (osal_clock_perf_reset(poller->perf) < 0) {
		ilerr__set("Could not reset performance counter");
		return IL_EFAIL;
	}

	/* start polling thread */
	poller->thread = osal_thread_create(poller_thread, poller);
	if (!poller->thread) {
		ilerr__set("Poller thread creation failed");
		return IL_EFAIL;
	}

	poller->running = 1;

	return 0;
}

void il_poller_stop(il_poller_t *poller)
{
	if (!poller)
		return;

	if (!poller->running)
		return;

	poller->stop = 1;
	osal_thread_join(poller->thread, NULL);

	poller->running = 0;
}

int il_poller_data_get(il_poller_t *poller, double **t_buf, double **d_buf,
		       size_t *cnt, int *lost)
{
	/* validate arguments */
	if (!poller) {
		ilerr__set("Invalid poller (NULL)");
		return IL_EFAULT;
	}

	if (!t_buf) {
		ilerr__set("Invalid time buffer pointer (NULL)");
		return IL_EFAULT;
	}

	if (!d_buf) {
		ilerr__set("Invalid data buffer pointer (NULL)");
		return IL_EFAULT;
	}

	if (!cnt) {
		ilerr__set("Invalid buffer data count pointer (NULL)");
		return IL_EFAULT;
	}

	osal_mutex_lock(poller->buf_lock);

	*t_buf = poller->t_buf[poller->buf_curr];
	*d_buf = poller->d_buf[poller->buf_curr];
	*cnt = poller->buf_cnt;

	if (lost)
		*lost = poller->lost;

	/* switch buffers, reset counters */
	poller->buf_curr = poller->buf_curr ? 0 : 1;
	poller->buf_cnt = 0;
	poller->lost = 0;

	osal_mutex_unlock(poller->buf_lock);

	return 0;
}
