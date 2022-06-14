#ifndef OSAL_POSIX_MUTEX_H_
#define OSAL_POSIX_MUTEX_H_

#include "osal/mutex.h"

#include <pthread.h>

/** Mutex (POSIX). */
struct osal_mutex {
	/** pthread mutex */
	pthread_mutex_t m;
};

#endif
