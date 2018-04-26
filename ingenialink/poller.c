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
#include "ingenialink/servo.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

int poller_td(void *args)
{
	il_poller_t *poller = args;
	osal_timespec_t curr;

	while (!poller->stop) {
		il_poller_acq_t *acq;
		double t;

		/* wait until next period */
		osal_timer_wait(poller->timer);

		/* obtain current time */
		osal_clock_perf_get(poller->perf, &curr);
		t = (double)curr.s + (double)curr.ns / 1000000000.;

		/* acquire all configured channels */
		osal_mutex_lock(poller->lock);

		acq = &poller->acq[poller->acq_curr];

		if (acq->cnt >= poller->sz) {
			acq->lost = 1;
		} else {
			size_t ch;

			acq->t[acq->cnt] = t;

			for (ch = 0; ch < poller->n_ch; ch++) {
				if (!poller->mappings_valid[ch])
					continue;

				(void)il_servo_read(poller->servo,
						    &poller->mappings[ch],
						    NULL,
						    &acq->d[ch][acq->cnt]);
			}

			acq->cnt++;
		}

		osal_mutex_unlock(poller->lock);
	}

	return 0;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_poller_t *il_poller_create(il_servo_t *servo, size_t n_ch)
{
	il_poller_t *poller;

	poller = calloc(1, sizeof(*poller));
	if (!poller) {
		ilerr__set("Poller allocation failed");
		return NULL;
	}

	poller->servo = servo;
	il_servo__retain(poller->servo);
	poller->n_ch = n_ch;

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

	poller->lock = osal_mutex_create();
	if (!poller->lock) {
		ilerr__set("Poller lock allocation failed");
		goto cleanup_perf;
	}

	poller->mappings = calloc(n_ch, sizeof(*poller->mappings));
	if (!poller->mappings) {
		ilerr__set("Poller mappings allocation failed");
		goto cleanup_lock;
	}

	poller->mappings_valid = calloc(n_ch, sizeof(*poller->mappings_valid));
	if (!poller->mappings_valid) {
		ilerr__set("Poller mappings valid allocation failed");
		goto cleanup_mappings;
	}

	poller->acq[0].d = calloc(n_ch, sizeof(*poller->acq[0].d));
	if (!poller->acq[0].d) {
		ilerr__set("Poller acquisition data allocation failed");
		goto cleanup_mappings_valid;
	}

	poller->acq[1].d = calloc(n_ch, sizeof(*poller->acq[1].d));
	if (!poller->acq[1].d) {
		ilerr__set("Poller acquisition data allocation failed");
		goto cleanup_acq_d_0;
	}

	return poller;

cleanup_acq_d_0:
	free(poller->acq[0].d);

cleanup_mappings_valid:
	free(poller->mappings_valid);

cleanup_mappings:
	free(poller->mappings);

cleanup_lock:
	osal_mutex_destroy(poller->lock);

cleanup_perf:
	osal_clock_perf_destroy(poller->perf);

cleanup_timer:
	osal_timer_destroy(poller->timer);

cleanup_poller:
	il_servo__release(poller->servo);
	free(poller);

	return NULL;
}

void il_poller_destroy(il_poller_t *poller)
{
	int i;

	if (poller->running)
		il_poller_stop(poller);

	for (i = 0; i < 2; i++) {
		size_t ch;
		il_poller_acq_t *acq = &poller->acq[i];

		if (acq->t)
			free(acq->t);

		for (ch = 0; ch < poller->n_ch; ch++) {
			if (acq->d[ch])
				free(acq->d[ch]);
		}
	}

	free(poller->acq[1].d);
	free(poller->acq[0].d);

	free(poller->mappings);
	free(poller->mappings_valid);

	osal_mutex_destroy(poller->lock);

	osal_clock_perf_destroy(poller->perf);
	osal_timer_destroy(poller->timer);

	il_servo__release(poller->servo);

	free(poller);
}

int il_poller_start(il_poller_t *poller)
{
	if (poller->running) {
		ilerr__set("Poller already running");
		return IL_EALREADY;
	}

	/* activate timer, reset performance counter */
	if (osal_timer_set(poller->timer,
			   poller->t_s * OSAL_TIMER_NANOSPERMSEC) < 0) {
		ilerr__set("Timer activation failed");
		return IL_EFAIL;
	}

	if (osal_clock_perf_reset(poller->perf) < 0) {
		ilerr__set("Performance counter reset failed");
		return IL_EFAIL;
	}

	/* start polling thread */
	poller->acq[poller->acq_curr].cnt = 0;
	poller->acq[poller->acq_curr].lost = 0;

	poller->stop = 0;

	poller->td = osal_thread_create(poller_td, poller);
	if (!poller->td) {
		ilerr__set("Poller thread creation failed");
		return IL_EFAIL;
	}

	poller->running = 1;

	return 0;
}

void il_poller_stop(il_poller_t *poller)
{
	if (!poller->running)
		return;

	poller->stop = 1;
	osal_thread_join(poller->td, NULL);

	poller->running = 0;
}

void il_poller_data_get(il_poller_t *poller, il_poller_acq_t **acq)
{
	osal_mutex_lock(poller->lock);

	*acq = &poller->acq[poller->acq_curr];

	poller->acq_curr = poller->acq_curr ? 0 : 1;
	poller->acq[poller->acq_curr].cnt = 0;
	poller->acq[poller->acq_curr].lost = 0;

	osal_mutex_unlock(poller->lock);
}

int il_poller_configure(il_poller_t *poller, unsigned int t_s, size_t sz)
{
	int i;

	if (poller->running) {
		ilerr__set("Poller is running");
		return IL_ESTATE;
	}

	for (i = 0; i < 2; i++) {
		size_t ch;
		il_poller_acq_t *acq = &poller->acq[i];

		acq->t = realloc(acq->t, sz * sizeof(*acq->t));
		if (!acq->t) {
			ilerr__set("Time buffer allocation failed");
			return IL_ENOMEM;
		}

		for (ch = 0; ch < poller->n_ch; ch++) {
			acq->d[ch] = realloc(acq->d[ch],
					     sz * sizeof(*acq->d[ch]));
			if (!acq->d[ch]) {
				ilerr__set("Data buffer allocation failed");
				return IL_ENOMEM;
			}
		}
	}

	poller->t_s = t_s;
	poller->sz = sz;

	return 0;
}

int il_poller_ch_configure(il_poller_t *poller, unsigned int ch,
			   const il_reg_t *reg, const char *id)
{
	const il_reg_t *reg_;

	if (poller->running) {
		ilerr__set("Poller is running");
		return IL_ESTATE;
	}

	if (ch >= poller->n_ch) {
		ilerr__set("Channel out of range");
		return IL_EINVAL;
	}

	/* obtain register */
	if (reg) {
		reg_ = reg;
	} else {
		int r;
		il_dict_t *dict;

		dict = il_servo_dict_get(poller->servo);
		if (!dict) {
			ilerr__set("No dictionary loaded");
			return IL_EFAIL;
		}

		r = il_dict_reg_get(dict, id, &reg_);
		if (r < 0)
			return r;
	}

	/* keep a copy of the register */
	memcpy(&poller->mappings[ch], reg_, sizeof(*reg_));
	poller->mappings_valid[ch] = 1;

	return 0;
}

int il_poller_ch_disable(il_poller_t *poller, unsigned int ch)
{
	if (poller->running) {
		ilerr__set("Poller is running");
		return IL_ESTATE;
	}

	if (ch >= poller->n_ch) {
		ilerr__set("Channel out of range");
		return IL_EINVAL;
	}

	poller->mappings_valid[ch] = 0;

	return 0;
}

int il_poller_ch_disable_all(il_poller_t *poller)
{
	size_t ch;

	for (ch = 0; ch < poller->n_ch; ch++) {
		int r;

		r = il_poller_ch_disable(poller, ch);
		if (r < 0)
			return r;
	}

	return 0;
}

