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

#include "utils.h"

#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Internal
 ******************************************************************************/

refcnt_t *refcnt__create(refcnt_destroy_t destroy, void *ctx)
{
	refcnt_t *refcnt;

	refcnt = malloc(sizeof(*refcnt));
	if (!refcnt) {
		ilerr__set("Reference counter allocation failed");
		return NULL;
	}

	refcnt->lock = osal_mutex_create();
	if (!refcnt->lock) {
		ilerr__set("Reference counter lock creation failed");
		goto cleanup;
	}

	refcnt->destroy = destroy;
	refcnt->ctx = ctx;
	refcnt->cnt = 1;

	return refcnt;

cleanup:
	free(refcnt);

	return NULL;
}

void refcnt__destroy(refcnt_t *refcnt)
{
	osal_mutex_destroy(refcnt->lock);
	free(refcnt);
}

void refcnt__retain(refcnt_t *refcnt)
{
	osal_mutex_lock(refcnt->lock);
	refcnt->cnt++;
	osal_mutex_unlock(refcnt->lock);
}

void refcnt__release(refcnt_t *refcnt)
{
	int release = 0;

	osal_mutex_lock(refcnt->lock);
	refcnt->cnt--;
	if (refcnt->cnt == 0)
		release = 1;
	osal_mutex_unlock(refcnt->lock);

	if (release) {
		refcnt->destroy(refcnt->ctx);
		refcnt__destroy(refcnt);
	}
}
