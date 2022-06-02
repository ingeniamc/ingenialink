#ifndef OSAL_WIN_CLOCK_H_
#define OSAL_WIN_CLOCK_H_

#include "osal/clock.h"

#include <Windows.h>

/** Performance counter (Windows). */
struct osal_clock_perf {
	/** QPC frequency. */
	LARGE_INTEGER freq;
	/** Reference time. */
	LARGE_INTEGER start;
};

#endif

