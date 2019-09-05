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

#ifndef OSAL_THREAD_H_
#define OSAL_THREAD_H_

/** Thread. */
typedef struct osal_thread osal_thread_t;

/** Thread function prototype. */
typedef int (*osal_thread_func_t)(void *args);

/**
 * Create a thread.
 *
 * @param [in] func
 *	Thread function.
 * @param [in] args
 *	Arguments passed to the thread.
 *
 * @return
 *	Thread (NULL if it could not be created).
 */
osal_thread_t *osal_thread_create_(osal_thread_func_t func, void *args);

/**
 * Join a thread (and destroy it).
 *
 * @param [in] thread
 *	Valid Thread.
 * @param [out] result
 *	Thread return code (optional).
 */
void osal_thread_join(osal_thread_t *thread, int *result);

#endif
