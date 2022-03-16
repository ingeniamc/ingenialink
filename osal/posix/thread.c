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

#include "thread.h"

#include <stdlib.h>
#include <string.h>

#include "osal/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Thread wrapper.
 *
 * @param [in] args
 *      Arguments (osal_thread_t *).
 *
 * @return
 *      NULL.
 */
static void *thread_wrapper(void *args)
{
	osal_thread_t *thread = args;

	thread->result = thread->func(thread->args);
	thread->isFinished = 1;

	return NULL;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_thread_t *osal_thread_create_(osal_thread_func_t func, void *args)
{
	int r;
	osal_thread_t *thread;

	thread = malloc(sizeof(*thread));
	if (!thread)
		return NULL;

	thread->func = func;
	thread->args = args;
	thread->result = 0;
	thread->isFinished = 0;

	r = pthread_create(&thread->t, NULL, thread_wrapper, thread);
	if (r)
		goto cleanup_thread;

	return thread;

cleanup_thread:
	free(thread);

	return NULL;
}

void osal_thread_join(osal_thread_t *thread, int *result)
{
	if (thread->isFinished != 1) {
		(void)pthread_join(thread->t, NULL);
	}

	if (result)
		*result = thread->result;

	free(thread);
}
