#ifndef OSAL_WIN_THREAD_H_
#define OSAL_WIN_THREAD_H_

#include "osal/thread.h"

#include <Windows.h>

/** Thread (Windows). */
struct osal_thread {
	/** Thread. */
	HANDLE t;
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

