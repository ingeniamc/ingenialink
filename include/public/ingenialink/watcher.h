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

#ifndef PUBLIC_INGENIALINK_WATCHER_H_
#define PUBLIC_INGENIALINK_WATCHER_H_

#include "axis.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/watcher.h
 * @brief Watcher.
 * @defgroup IL_WATCHER Watcher
 * @ingroup IL
 * @{
 */

/** IngeniaLink watcher. */
typedef struct il_watcher il_watcher_t;

/** Watcher subcriber callback. */
typedef void (*il_watcher_subscriber_cb_t)(void *ctx, double value);

/**
 * Create a register watcher.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 * @param [in] base_period
 *	Base watching period (ms).
 *
 * @return
 *	Watcher instance (NULL if it could not be created).
 */
IL_EXPORT il_watcher_t *il_watcher_create(il_axis_t *axis, int base_period);

/**
 * Destroy a register watcher.
 *
 * @param [in] watcher
 *	Watcher instance.
 */
IL_EXPORT void il_watcher_destroy(il_watcher_t *watcher);

/**
 * Start watcher.
 *
 * @param [in] watcher
 *	Watcher instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_watcher_start(il_watcher_t *watcher);

/**
 * Stop watcher.
 *
 * @param [in] watcher
 *	Watcher instance.
 */
IL_EXPORT void il_watcher_stop(il_watcher_t *watcher);

/**
 * Subscribe to changes on a register.
 *
 * @param [in] watcher
 *	Watcher instance.
 * @param [in] reg
 *	Register.
 * @param [in] period
 *	Watching period (ms).
 * @param [in] cb
 *	Value change callback.
 * @param [in] ctx
 *	Value change callback context (optional).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_watcher_subscribe(il_watcher_t *watcher, const il_reg_t *reg,
				   int period, il_watcher_subscriber_cb_t cb,
				   void *ctx);

/**
 * Unsubcribe from changes on a register.
 *
 * @param [in] watcher
 *	Watcher instance.
 * @param [in] reg
 *	Register.
 */
IL_EXPORT void il_watcher_unsubscribe(il_watcher_t *watcher,
				      const il_reg_t *reg);

/** @} */

IL_END_DECL

#endif
