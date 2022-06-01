#include "utils.h"

#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Internal
 ******************************************************************************/

il_utils_refcnt_t *il_utils__refcnt_create(il_utils_refcnt_destroy_t destroy,
					   void *ctx)
{
	il_utils_refcnt_t *refcnt;

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

void il_utils__refcnt_destroy(il_utils_refcnt_t *refcnt)
{
	osal_mutex_destroy(refcnt->lock);
	free(refcnt);
}

void il_utils__refcnt_retain(il_utils_refcnt_t *refcnt)
{
	osal_mutex_lock(refcnt->lock);
	refcnt->cnt++;
	osal_mutex_unlock(refcnt->lock);
}

void il_utils__refcnt_release(il_utils_refcnt_t *refcnt)
{
	int release = 0;

	osal_mutex_lock(refcnt->lock);
	refcnt->cnt--;
	if (refcnt->cnt == 0)
		release = 1;
	osal_mutex_unlock(refcnt->lock);

	if (release) {
		refcnt->destroy(refcnt->ctx);
		il_utils__refcnt_destroy(refcnt);
	}
}
