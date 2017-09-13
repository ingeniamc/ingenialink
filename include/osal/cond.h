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

#ifndef OSAL_COND_H_
#define OSAL_COND_H_

#include "mutex.h"

/** Condition variable. */
typedef struct osal_cond osal_cond_t;

/**
 * Create a condition variable.
 *
 * @return
 *	Condition variable (NULL if it could not be created).
 *
 * @see
 *	osal_cond_destroy
 */
osal_cond_t *osal_cond_create(void);

/**
 * Destroy a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 *
 * @see
 *	osal_cond_init
 */
void osal_cond_destroy(osal_cond_t *cond);

/**
 * Wait for a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 * @param [in] mutex
 *	Valid mutex, used to enter the critical section.
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @return
 *      0 on successful acquisition, error code otherwise.
 */
int osal_cond_wait(osal_cond_t *cond, osal_mutex_t *mutex, int timeout);

/**
 * Signal a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 */
void osal_cond_signal(osal_cond_t *cond);

/**
 * Signal all condition variables.
 *
 * @param [in] cond
 *	Valid condition variable.
 */
void osal_cond_broadcast(osal_cond_t *cond);

#endif
