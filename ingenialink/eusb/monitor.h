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

#ifndef MONITOR_H_
#define MONITOR_H_

#include "public/ingenialink/monitor.h"

#include "osal/osal.h"

/** Monitoring base period (uS). */
#define BASE_PERIOD		100

/** Monitor mapping, index offset. */
#define MAPPING_IDX_OFFSET	16
/** Monitor mapping, sub-index offset. */
#define MAPPING_SIDX_OFFSET	8

/** Monitor trigger source, sub-index offset. */
#define TRIGSRC_SIDX_OFFSET	16

/** Availability wait time (ms) */
#define AVAILABLE_WAIT_TIME	100

/** Acquisition context. */
typedef struct {
	/** Acquisition (uses double buffering mechanism). */
	il_monitor_acq_t acq[2];
	/** Current acquisition. */
	int curr;
	/** Size. */
	size_t sz;
	/** Sampling period (s). */
	double t_s;
	/** Maximum number of samples. */
	size_t max_samples;
	/** Lock. */
	osal_mutex_t *lock;
	/** Finished condition. */
	osal_cond_t *finished_cond;
	/** Finished flag. */
	int finished;
	/** Thread. */
	osal_thread_t *td;
	/** Stop flag. */
	int stop;
} il_monitor_acq_ctx_t;

/** IngeniaLink monitor. */
struct il_monitor {
	/** Associated servo. */
	il_servo_t *servo;
	/** Mapped registers. */
	const il_reg_t *mappings[IL_MONITOR_CH_NUM];
	/** Acquisition context. */
	il_monitor_acq_ctx_t acq;
};

#endif
