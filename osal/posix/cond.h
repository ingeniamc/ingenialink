#ifndef OSAL_POSIX_COND_H_
#define OSAL_POSIX_COND_H_

#include "osal/cond.h"

#include <pthread.h>

/** Condition variable (POSIX). */
struct osal_cond {
	/** Condition variable. */
	pthread_cond_t c;
};

#endif
