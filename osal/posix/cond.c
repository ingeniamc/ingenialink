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

#include "cond.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "mutex.h"
#include "osal/err.h"

/*
 * TODO: May use CLOCK_MONOTONIC on Linux.
 */

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_cond_t *osal_cond_create()
{
	int r;
	osal_cond_t *cond;

	cond = malloc(sizeof(*cond));
	if (!cond)
		return NULL;

	r = pthread_cond_init(&cond->c, NULL);
	if (r)
		goto cleanup_cond;

	return cond;

cleanup_cond:
	free(cond);

	return NULL;
}

void osal_cond_destroy(osal_cond_t *cond)
{
	(void)pthread_cond_destroy(&cond->c);
	free(cond);
}

int osal_cond_wait(osal_cond_t *cond, osal_mutex_t *mutex, int timeout)
{
	int r;

	if (timeout <= 0) {
		r = pthread_cond_wait(&cond->c, &mutex->m);
		if (r)
			return OSAL_EFAIL;
	} else {
		struct timespec ts;
		struct timeval tv;

		if (gettimeofday(&tv, NULL))
			return OSAL_EFAIL;

		ts.tv_nsec = tv.tv_usec * 1000 + timeout * 1000000L;
		ts.tv_sec = tv.tv_sec + ts.tv_nsec / 1000000000L;
		ts.tv_nsec = ts.tv_nsec % 1000000000L;

		r = pthread_cond_timedwait(&cond->c, &mutex->m, &ts);
		if (r) {
			if (r == ETIMEDOUT)
				return OSAL_ETIMEDOUT;

			return OSAL_EFAIL;
		}
	}

	return 0;
}

void osal_cond_signal(osal_cond_t *cond)
{
	(void)pthread_cond_signal(&cond->c);
}

void osal_cond_broadcast(osal_cond_t *cond)
{
	(void)pthread_cond_broadcast(&cond->c);
}
