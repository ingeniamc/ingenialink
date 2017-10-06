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

#ifndef PUBLIC_INGENIALINK_POLLER_H_
#define PUBLIC_INGENIALINK_POLLER_H_

#include "servo.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/poller.h
 * @brief Poller.
 * @defgroup IL_POLLER Poller
 * @ingroup IL
 * @{
 */

/** IngeniaLink poller. */
typedef struct il_poller il_poller_t;

/** Poller acquisition results. */
typedef struct {
	/** Time vector. */
	double *t;
	/** Data vectors. */
	double **d;
	/** Number of actual samples in the time and data vectors. */
	size_t cnt;
	/** Data lost flag. */
	int lost;
} il_poller_acq_t;

/**
 * Create a register poller.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] n_ch
 *	Number of channels.
 *
 * @return
 *	Poller instance (NULL if it could not be created).
 */
IL_EXPORT il_poller_t *il_poller_create(il_servo_t *servo, size_t n_ch);

/**
 * Destroy a register poller.
 *
 * @param [in] poller
 *	Poller instance.
 */
IL_EXPORT void il_poller_destroy(il_poller_t *poller);

/**
 * Start poller.
 *
 * @param [in] poller
 *	Poller instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_start(il_poller_t *poller);

/**
 * Stop poller.
 *
 * @param [in] poller
 *	Poller instance.
 */
IL_EXPORT void il_poller_stop(il_poller_t *poller);

/**
 * Obtain current time and data vectors.
 *
 * @note
 *	The obtained acquisition data can be used until the next call to this
 *	function.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [out] acq
 *	Where the acquisition results will be left.
 */
IL_EXPORT void il_poller_data_get(il_poller_t *poller, il_poller_acq_t **acq);

/**
 * Configure poller parameters.
 *
 * @note
 *	- The maximum stable polling rate is ~500 Hz (2 ms) for a single
 *	  register when the servo is enabled.
 *	- The buffer size must be set according to your application needs. It
 *	  should be large enough so that it can store all samples collected
 *	  between subsequent calls to `il_poller_data_get`.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] t_s
 *	Sampling period (ms).
 * @param [in] buf_sz
 *	Buffer size.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_configure(il_poller_t *poller, unsigned int t_s,
				  size_t buf_sz);

/**
 * Configure a poller channel.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] ch
 *	Channel.
 * @param [in] reg
 *	Register to be polled on this channel.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_configure(il_poller_t *poller, unsigned int ch,
				     const il_reg_t *reg);

/**
 * Disable a poller channel.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] ch
 *	Channel.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_disable(il_poller_t *poller, unsigned int ch);

/**
 * Disable all poller channels.
 *
 * @param [in] poller
 *	Poller instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_disable_all(il_poller_t *poller);

/** @} */

IL_END_DECL

#endif
