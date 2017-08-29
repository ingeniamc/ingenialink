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

#ifndef OSAL_MUTEX_H_
#define OSAL_MUTEX_H_

/** Mutex. */
typedef struct osal_mutex osal_mutex_t;

/**
 * Create a mutex.
 *
 * @return
 *      Mutex (NULL if it could not be created).
 *
 * @see
 *      osal_mutex_destroy
 */
osal_mutex_t *osal_mutex_create(void);

/**
 * Destroy a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_create
 */
void osal_mutex_destroy(osal_mutex_t *mutex);

/**
 * Acquire a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_unlock
 */
void osal_mutex_lock(osal_mutex_t *mutex);

/**
 * Release a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_lock
 */
void osal_mutex_unlock(osal_mutex_t *mutex);

#endif
