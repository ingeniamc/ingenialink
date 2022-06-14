#ifndef OSAL_WIN_MUTEX_H_
#define OSAL_WIN_MUTEX_H_

#include "osal/mutex.h"

#include <Windows.h>

/** Mutex (Windows). */
struct osal_mutex {
	/** Critical section. */
	CRITICAL_SECTION m;
};

#endif
