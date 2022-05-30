#ifndef UTILS_H_
#define UTILS_H_

#include "ingenialink/utils.h"

#include <osal/osal.h>

/** Reference counter. */
struct il_utils_refcnt {
	/** De-allocation callback. */
	il_utils_refcnt_destroy_t destroy;
	/** Callback context. */
	void *ctx;
	/** Lock. */
	osal_mutex_t *lock;
	/** Counter. */
	int cnt;
};

#endif
