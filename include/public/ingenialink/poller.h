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

#include "axis.h"

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

/**
 * Create a register poller.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 * @param [in] reg
 *	Register to be polled.
 * @param [in] period
 *	Polling period (ms).
 * @param [in] sz
 *	Buffer size.
 *
 * @return
 *	Poller instance (NULL if it could not be created).
 */
IL_EXPORT il_poller_t *il_poller_create(il_axis_t *axis, const il_reg_t *reg,
					int period, size_t sz);

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
 * Obtain a pointer to the current.
 *
 * @note
 *	Pollers use a double buffering mechanism, so you can access the data
 *	contained in the buffer safely.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [out] t_buf
 *	Time vector buffer.
 * @param [out] d_buf
 *	Data vector buffer.
 * @param [out] cnt
 *	Number of samples in the buffers.
 * @param [out] lost
 *	Will be set to 1 if data was lost (optional).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_data_get(il_poller_t *poller, double **t_buf,
				 double **d_buf, size_t *cnt, int *lost);

/** @} */

IL_END_DECL

#endif
