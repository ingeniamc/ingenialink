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

#ifndef WATCHER_H_
#define WATCHER_H_

#include "public/ingenialink/watcher.h"

#include "osal/osal.h"

/** Default subscribers list size. */
#define SUBS_SZ_DEF	10

/** IngeniaLink watcher subscriber. */
typedef struct {
	/** Register. */
	const il_reg_t *reg;
	/** Period. */
	int period;
	/** Time. */
	int time;
	/** Current value. */
	double value;
	/** Callback. */
	il_watcher_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
} il_watcher_subscriber_t;

/**
 * IngeniaLink watcher subscribers.
 *
 * @note
 *	This is implemented using a dynamic array so that traverse is more
 *	efficient.
 */
typedef struct {
	/** Array of subscribers. */
	il_watcher_subscriber_t *subs;
	/** Array size. */
	size_t sz;
	/** Number of subscribers. */
	size_t cnt;
	/** Lock. */
	osal_mutex_t *lock;
} il_watcher_subscriber_lst_t;

/** IngeniaLink register watcher. */
struct il_watcher {
	/** Associated axis. */
	il_axis_t *axis;
	/** Stop flag. */
	int stop;
	/** Running flag. */
	int running;
	/** Timer. */
	osal_timer_t *timer;
	/** Watcher thread. */
	osal_thread_t *thread;
	/** Base period (ms). */
	int base_period;
	/** Subcribers. */
	il_watcher_subscriber_lst_t subs;
};

#endif
