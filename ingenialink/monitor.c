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

#include "monitor.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ingenialink/err.h"
#include "ingenialink/registers.h"
#include "ingenialink/utils.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/** Results registers map. */
static const il_reg_t *result_regs[] = {
	&IL_REG_MONITOR_RESULT_CH_1,
	&IL_REG_MONITOR_RESULT_CH_2,
	&IL_REG_MONITOR_RESULT_CH_3,
	&IL_REG_MONITOR_RESULT_CH_4
};

/** Mapping registers map. */
static const il_reg_t *map_regs[] = {
	&IL_REG_MONITOR_MAP_CH_1,
	&IL_REG_MONITOR_MAP_CH_2,
	&IL_REG_MONITOR_MAP_CH_3,
	&IL_REG_MONITOR_MAP_CH_4
};

/**
 * Check if the current acquisition has finished.
 *
 * @param [in] monitor
 *	Monitor instance.
 *
 * @return
 *	1 if finished, 0 if not.
 */
static int acquisition_has_finished(il_monitor_t *monitor)
{
	int r;

	osal_mutex_lock(monitor->acq.lock);

	r = monitor->acq.finished;

	osal_mutex_unlock(monitor->acq.lock);

	return r;
}

/**
 * Acquisition thread
 *
 * @param [in] args
 *	Thread arguments (il_monitor_t *).
 */
static int acquisition(void *args)
{
	il_monitor_t *monitor = args;

	int r = 0;
	uint16_t acquired = 0;
	int ch;
	double scalings[IL_MONITOR_CH_NUM];

	/* obtain units factors */
	for (ch = 0; ch < IL_MONITOR_CH_NUM; ch++) {
		if (!monitor->mappings[ch])
			continue;

		scalings[ch] = il_servo_units_factor(monitor->servo,
						     monitor->mappings[ch]);
	}

	/* acquire */
	while (!monitor->acq.stop && (acquired < monitor->acq.sz)) {
		uint16_t available = 0;

		/* obtain number of available samples */
		r = il_servo_raw_read_u16(monitor->servo,
					  &IL_REG_MONITOR_RESULT_FILLED,
					  &available);
		if (r < 0)
			goto out;

		/* clip available (user may have requested less) */
		available = MIN(available, monitor->acq.sz);

		/* read available samples */
		while (!monitor->acq.stop && (acquired < available)) {
			il_monitor_acq_t *acq;

			/* set index */
			r = il_servo_raw_write_u16(monitor->servo,
						   &IL_REG_MONITOR_RESULT_ENTRY,
						   acquired);
			if (r < 0)
				goto out;

			/* obtain samples for each configured channel */
			osal_mutex_lock(monitor->acq.lock);

			acq = &monitor->acq.acq[monitor->acq.curr];

			for (ch = 0; ch < IL_MONITOR_CH_NUM; ch++) {
				int32_t value;

				if (!monitor->mappings[ch])
					continue;

				r = il_servo_raw_read_s32(monitor->servo,
							  result_regs[ch],
							  &value);
				if (r < 0) {
					osal_mutex_unlock(monitor->acq.lock);
					goto out;
				}

				acq->samples[ch][acq->n_samples] =
					(double)value * scalings[ch];
			}

			acq->n_samples++;

			osal_mutex_unlock(monitor->acq.lock);

			acquired++;
		}
	}

out:
	/* disable monitor */
	(void)il_servo_raw_write_u8(
			monitor->servo, &IL_REG_MONITOR_CFG_ENABLE, 0);

	/* signal finished */
	osal_mutex_lock(monitor->acq.lock);
	monitor->acq.finished = 1;
	osal_cond_signal(monitor->acq.finished_cond);
	osal_mutex_unlock(monitor->acq.lock);

	return r;
}

/**
 * Update data buffers
 *
 * @param [in] monitor
 *	Monitor instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int update_buffers(il_monitor_t *monitor)
{
	int r;
	uint16_t sz;
	int ch, i;

	/* update current size */
	r = il_servo_raw_read_u16(
			monitor->servo, &IL_REG_MONITOR_RESULT_SZ, &sz);
	if (r < 0)
		return r;

	monitor->acq.sz = (size_t)MIN(monitor->acq.max_samples, sz);

	monitor->acq.acq[0].sz = monitor->acq.sz;
	monitor->acq.acq[1].sz = monitor->acq.sz;

	/* reallocate (or free) double-buffers */
	for (i = 0; i < 2; i++) {
		il_monitor_acq_t *acq = &monitor->acq.acq[i];

		for (ch = 0; ch < IL_MONITOR_CH_NUM; ch++) {
			if (!monitor->mappings[ch]) {
				if (acq->samples[ch]) {
					free(acq->samples[ch]);
					acq->samples[ch] = NULL;
				}
			} else {
				acq->samples[ch] = realloc(
					acq->samples[ch],
					sizeof(*acq->samples) * sz);
				if (!acq->samples[ch]) {
					ilerr__set("Buffer allocation failed");
					return IL_ENOMEM;
				}
			}
		}
	}

	return 0;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_monitor_t *il_monitor_create(il_servo_t *servo)
{
	il_monitor_t *monitor;

	monitor = calloc(1, sizeof(*monitor));
	if (!monitor) {
		ilerr__set("Monitor allocation failed");
		return NULL;
	}

	/* setup acquisition resources */
	monitor->acq.lock = osal_mutex_create();
	if (!monitor->acq.lock) {
		ilerr__set("Acquisiton lock creation failed");
		goto cleanup_monitor;
	}

	monitor->acq.finished_cond = osal_cond_create();
	if (!monitor->acq.finished_cond) {
		ilerr__set("Acquisition finished condition creation failed");
		goto cleanup_acq_lock;
	}

	monitor->acq.finished = 1;

	monitor->servo = servo;

	return monitor;

cleanup_acq_lock:
	osal_mutex_destroy(monitor->acq.lock);

cleanup_monitor:
	free(monitor);

	return NULL;
}

void il_monitor_destroy(il_monitor_t *monitor)
{
	int i, ch;

	assert(monitor);

	il_monitor_stop(monitor);

	osal_cond_destroy(monitor->acq.finished_cond);
	osal_mutex_destroy(monitor->acq.lock);

	for (i = 0; i < 2; i++) {
		il_monitor_acq_t *acq = &monitor->acq.acq[i];

		for (ch = 0; ch < IL_MONITOR_CH_NUM; ch++) {
			if (acq->samples[ch])
				free(acq->samples[ch]);
		}
	}

	free(monitor);
}

int il_monitor_start(il_monitor_t *monitor)
{
	int r;

	assert(monitor);

	if (!acquisition_has_finished(monitor)) {
		ilerr__set("Acquisition already in progress");
		return IL_EALREADY;
	}

	/* clear previous acquisition resources */
	il_monitor_stop(monitor);

	/* enable monitoring (0 -> 1) */
	r = il_servo_raw_write_u8(
			monitor->servo, &IL_REG_MONITOR_CFG_ENABLE, 0);
	if (r < 0)
		return r;

	r = il_servo_raw_write_u8(
			monitor->servo, &IL_REG_MONITOR_CFG_ENABLE, 1);
	if (r < 0)
		return r;

	/* launch acquisition */
	monitor->acq.stop = 0;
	monitor->acq.finished = 0;

	monitor->acq.td = osal_thread_create(acquisition, monitor);
	if (!monitor->acq.td) {
		monitor->acq.finished = 1;

		ilerr__set("Acquisition thread creation failed");
		return IL_EFAIL;
	}

	return 0;
}

void il_monitor_stop(il_monitor_t *monitor)
{
	assert(monitor);

	if (monitor->acq.td) {
		monitor->acq.stop = 1;
		osal_thread_join(monitor->acq.td, NULL);
		monitor->acq.td = NULL;
	}
}

int il_monitor_wait(il_monitor_t *monitor, int timeout)
{
	int r = 0;

	assert(monitor);

	osal_mutex_lock(monitor->acq.lock);

	if (!monitor->acq.finished) {
		r = osal_cond_wait(monitor->acq.finished_cond,
				   monitor->acq.lock, timeout);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Acquisition completion timed out");
			r = IL_ETIMEDOUT;
		} else if (r < 0) {
			ilerr__set("Acquisition completion wait failed");
			r = IL_EFAIL;
		}
	}

	osal_mutex_unlock(monitor->acq.lock);

	return r;
}

void il_monitor_data_get(il_monitor_t *monitor, il_monitor_acq_t **acq)
{
	assert(monitor);
	assert(acq);

	osal_mutex_lock(monitor->acq.lock);

	*acq = &monitor->acq.acq[monitor->acq.curr];

	monitor->acq.curr = monitor->acq.curr ? 0 : 1;
	monitor->acq.acq[monitor->acq.curr].n_samples = 0;

	osal_mutex_unlock(monitor->acq.lock);
}

int il_monitor_configure(il_monitor_t *monitor, unsigned int t_s,
			 size_t delay_samples, size_t max_samples)
{
	int r;

	assert(monitor);

	if (!acquisition_has_finished(monitor)) {
		ilerr__set("Acquisition in progress");
		return IL_ESTATE;
	}

	r = il_servo_raw_write_u16(monitor->servo, &IL_REG_MONITOR_CFG_T_S,
				   (uint16_t)(t_s / BASE_PERIOD));
	if (r < 0)
		return r;

	r = il_servo_raw_write_u32(monitor->servo,
				   &IL_REG_MONITOR_CFG_DELAY_SAMPLES,
				   (uint32_t)delay_samples);

	monitor->acq.max_samples = max_samples;

	return 0;
}

int il_monitor_ch_configure(il_monitor_t *monitor, il_monitor_ch_t ch,
			    const il_reg_t *reg)
{
	int r;

	uint8_t bits;
	int32_t mapping;

	assert(monitor);
	assert(reg);

	if (!acquisition_has_finished(monitor)) {
		ilerr__set("Acquisition in progress");
		return IL_ESTATE;
	}

	/* compute mapping, configure it */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
	case IL_REG_DTYPE_S8:
		bits = 8;
		break;
	case IL_REG_DTYPE_U16:
	case IL_REG_DTYPE_S16:
		bits = 16;
		break;
	case IL_REG_DTYPE_U32:
	case IL_REG_DTYPE_S32:
		bits = 32;
		break;
	default:
		bits = 64;
	}

	mapping = (reg->idx << MAPPING_IDX_OFFSET) |
		  (reg->sidx << MAPPING_SIDX_OFFSET) | bits;

	r = il_servo_raw_write_s32(monitor->servo, map_regs[ch], mapping);
	if (r < 0)
		return r;

	monitor->mappings[ch] = reg;

	return update_buffers(monitor);
}

int il_monitor_ch_disable(il_monitor_t *monitor, il_monitor_ch_t ch)
{
	int r;

	assert(monitor);

	if (!acquisition_has_finished(monitor)) {
		ilerr__set("Acquisition in progress");
		return IL_ESTATE;
	}

	r = il_servo_raw_write_s32(monitor->servo, map_regs[ch], 0);
	if (r < 0)
		return r;

	monitor->mappings[ch] = NULL;

	return update_buffers(monitor);
}

int il_monitor_ch_disable_all(il_monitor_t *monitor)
{
	il_monitor_ch_t ch;

	for (ch = IL_MONITOR_CH_1; ch <= IL_MONITOR_CH_4; ch++) {
		int r;

		r = il_monitor_ch_disable(monitor, ch);
		if (r < 0)
			return r;
	}

	return 0;
}

int il_monitor_trigger_configure(il_monitor_t *monitor,
				 il_monitor_trigger_t mode,
				 size_t delay_samples, const il_reg_t *source,
				 double th_pos, double th_neg, uint32_t din_msk)
{
	int r;

	assert(monitor);

	if (!acquisition_has_finished(monitor)) {
		ilerr__set("Acquisition in progress");
		return IL_ESTATE;
	}

	/* mode */
	r = il_servo_raw_write_u8(monitor->servo, &IL_REG_MONITOR_TRIG_MODE,
				  (uint8_t)mode);
	if (r < 0)
		return r;

	/* delay samples */
	r = il_servo_raw_write_u32(monitor->servo, &IL_REG_MONITOR_TRIG_DELAY,
				   (uint32_t)delay_samples);
	if (r < 0)
		return r;

	/* source register */
	if ((mode == IL_MONITOR_TRIGGER_POS) ||
	    (mode == IL_MONITOR_TRIGGER_NEG) ||
	    (mode == IL_MONITOR_TRIGGER_WINDOW)) {
		uint32_t src;

		assert(source);

		src = (source->sidx << TRIGSRC_SIDX_OFFSET) | source->idx;

		r = il_servo_raw_write_u32(
				monitor->servo, &IL_REG_MONITOR_TRIG_SRC, src);
		if (r < 0)
			return r;
	}

	/* positive threshold */
	if ((mode == IL_MONITOR_TRIGGER_POS) ||
	    (mode == IL_MONITOR_TRIGGER_WINDOW)) {
		int32_t th_pos_ = (int32_t)(
			th_pos / il_servo_units_factor(monitor->servo, source));

		r = il_servo_raw_write_s32(
				monitor->servo, &IL_REG_MONITOR_TRIG_TH_POS,
				th_pos_);
		if (r < 0)
			return r;
	}

	/* negative threshold */
	if ((mode == IL_MONITOR_TRIGGER_NEG) ||
	    (mode == IL_MONITOR_TRIGGER_WINDOW)) {
		int32_t th_neg_ = (int32_t)(
			th_neg / il_servo_units_factor(monitor->servo, source));

		r = il_servo_raw_write_s32(
				monitor->servo, &IL_REG_MONITOR_TRIG_TH_NEG,
				th_neg_);
		if (r < 0)
			return r;
	}

	/* digital input mask */
	if (mode == IL_MONITOR_TRIGGER_DIN) {
		r = il_servo_raw_write_u32(
				monitor->servo, &IL_REG_MONITOR_TRIG_DIN_MSK,
				din_msk);
		if (r < 0)
			return r;
	}

	return 0;
}
