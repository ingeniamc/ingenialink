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

#include "watcher.h"

#include <stdlib.h>
#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/*
 * Watcher thread
 *
 * @note
 *	As watcher only cares about value changes, timer overruns are ignored.
 *	It is users responsability to not overload the watcher.
 *
 * @param [in] args
 *	Thread arguments (il_watcher_t *).
 */
int watcher_thread(void *args)
{
	il_watcher_t *watcher = args;

	while (!watcher->stop) {
		size_t i;

		osal_timer_wait(watcher->timer);

		osal_mutex_lock(watcher->subs.lock);

		for (i = 0; i < watcher->subs.cnt; i++) {
			il_watcher_subscriber_t *subs = &watcher->subs.subs[i];

			subs->time += watcher->base_period;

			if (subs->time >= subs->period) {
				int r;
				double value;

				r = il_axis_read(watcher->axis, subs->reg,
						 &value);

				if ((r == 0) && (subs->value != value)) {
					subs->cb(subs->ctx, value);
					subs->value = value;
				}

				subs->time = 0;
			}
		}

		osal_mutex_unlock(watcher->subs.lock);
	}

	return 0;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_watcher_t *il_watcher_create(il_axis_t *axis, int base_period)
{
	il_watcher_t *watcher;

	watcher = malloc(sizeof(*watcher));
	if (!watcher) {
		ilerr__set("Watcher allocation failed");
		return NULL;
	}

	watcher->timer = osal_timer_create();
	if (!watcher->timer) {
		ilerr__set("Watcher timer allocation failed");
		goto cleanup_watcher;
	}

	/* initialize subscribers resources */
	watcher->subs.subs = malloc(sizeof(*watcher->subs.subs) * SUBS_SZ_DEF);
	if (!watcher->subs.subs) {
		ilerr__set("Watcher subscribers allocation failed");
		goto cleanup_timer;
	}

	watcher->subs.lock = osal_mutex_create();
	if (!watcher->subs.lock) {
		ilerr__set("Watcher subscribers lock allocation failed");
		goto cleanup_subs_subs;
	}

	watcher->subs.cnt = 0;
	watcher->subs.sz = SUBS_SZ_DEF;

	watcher->axis = axis;
	watcher->base_period = base_period;
	watcher->stop = 0;
	watcher->running = 0;

	return watcher;

cleanup_subs_subs:
	free(watcher->subs.subs);

cleanup_timer:
	osal_timer_destroy(watcher->timer);

cleanup_watcher:
	free(watcher);

	return NULL;
}

void il_watcher_destroy(il_watcher_t *watcher)
{
	if (watcher->running)
		il_watcher_stop(watcher);

	osal_mutex_destroy(watcher->subs.lock);
	free(watcher->subs.subs);

	osal_timer_destroy(watcher->timer);

	free(watcher);
}

int il_watcher_start(il_watcher_t *watcher)
{
	if (!watcher) {
		ilerr__set("Invalid watcher (NULL)");
		return IL_EFAULT;
	}

	if (watcher->running) {
		ilerr__set("Watcher already running");
		return IL_EALREADY;
	}

	/* activate timer, reset performance counter */
	if (osal_timer_set(
		watcher->timer,
		watcher->base_period * OSAL_TIMER_NANOSPERMSEC) < 0) {
		ilerr__set("Timer activation failed");
		return IL_EFAIL;
	}

	/* start watcher thread */
	watcher->thread = osal_thread_create(watcher_thread, watcher);
	if (!watcher->thread) {
		ilerr__set("Watcher thread creation failed");
		return IL_EFAIL;
	}

	watcher->running = 1;

	return 0;
}

void il_watcher_stop(il_watcher_t *watcher)
{
	if (!watcher)
		return;

	if (!watcher->running)
		return;

	watcher->stop = 1;
	osal_thread_join(watcher->thread, NULL);

	watcher->running = 0;
}

int il_watcher_subscribe(il_watcher_t *watcher, const il_reg_t *reg, int period,
			 il_watcher_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	size_t i;

	if (!watcher) {
		ilerr__set("Invalid watcher (NULL)");
		return IL_EFAULT;
	}

	if (!reg) {
		ilerr__set("Invalid register (NULL)");
		return IL_EFAULT;
	}

	osal_mutex_lock(watcher->subs.lock);

	/* check if already subscribed */
	for (i = 0; i < watcher->subs.cnt; i++) {
		if (watcher->subs.subs[i].reg == reg) {
			ilerr__set("Register already being watched");
			r = IL_EALREADY;
			goto unlock;
		}
	}

	/* increase array if no space left */
	if (watcher->subs.cnt == watcher->subs.sz) {
		size_t sz;
		il_watcher_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * watcher->subs.sz * sizeof(*watcher);
		subs = realloc(watcher->subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		watcher->subs.subs = subs;
		watcher->subs.sz = sz;
	}

	watcher->subs.subs[watcher->subs.cnt].reg = reg;
	watcher->subs.subs[watcher->subs.cnt].period = period;
	watcher->subs.subs[watcher->subs.cnt].time = 0;
	watcher->subs.subs[watcher->subs.cnt].value = 0.;
	watcher->subs.subs[watcher->subs.cnt].cb = cb;
	watcher->subs.subs[watcher->subs.cnt].ctx = ctx;

	watcher->subs.cnt++;

unlock:
	osal_mutex_unlock(watcher->subs.lock);

	return r;
}

void il_watcher_unsubscribe(il_watcher_t *watcher, const il_reg_t *reg)
{
	size_t i;

	if (!watcher)
		return;

	osal_mutex_lock(watcher->subs.lock);

	for (i = 0; i < watcher->subs.cnt; i++) {
		if (watcher->subs.subs[i].reg == reg) {
			/* move last to the current position, decrease */
			watcher->subs.subs[i] =
				watcher->subs.subs[watcher->subs.cnt - 1];
			watcher->subs.cnt--;
			break;
		}
	}

	osal_mutex_unlock(watcher->subs.lock);
}
