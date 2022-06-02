#ifndef OSAL_WIN_TIMER_H_
#define OSAL_WIN_TIMER_H_

#include "osal/timer.h"

#include <Windows.h>

/** Step size (ns) of the FILETIME values. */
#define STEP_SIZE	100U

/** Timer (Windows). */
struct osal_timer {
	/** Handle */
	HANDLE hnd;
};

#endif

