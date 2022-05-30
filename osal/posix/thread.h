#ifndef OSAL_POSIX_THREAD_H_
#define OSAL_POSIX_THREAD_H_

#include "osal/thread.h"

#include <pthread.h>

/** Thread (POSIX). */
struct osal_thread {
	/** Thread. */
	pthread_t t;
	/** Function. */
	osal_thread_func_t func;
	/** Arguments. */
	void *args;
	/** Thread return value. */
	int result;
	/** Thread is finished */
	int isFinished;
};

#endif

