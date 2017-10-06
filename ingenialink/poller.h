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

#ifndef POLLER_H_
#define POLLER_H_

#include "public/ingenialink/poller.h"

#include "osal/osal.h"

/** IngeniaLink register poller. */
struct il_poller {
	/** Associated servo. */
	il_servo_t *servo;
	/** Number of channels. */
	size_t n_ch;
	/** Mapped registers to each channel. */
	const il_reg_t **mappings;
	/** Acquisition (uses double buffering mechanism). */
	il_poller_acq_t acq[2];
	/** Current acquisition. */
	int acq_curr;
	/** Sampling period (ms). */
	int t_s;
	/** Buffer size. */
	size_t sz;
	/** Timer. */
	osal_timer_t *timer;
	/** Performance counter. */
	osal_clock_perf_t *perf;
	/** Lock. */
	osal_mutex_t *lock;
	/** Thread. */
	osal_thread_t *td;
	/** Running flag. */
	int running;
	/** Stop flag. */
	int stop;
};

#endif
