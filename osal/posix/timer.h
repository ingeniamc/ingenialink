#ifndef OSAL_POSIX_TIMER_H_
#define OSAL_POSIX_TIMER_H_

#include "osal/timer.h"

#if defined(__MACH__) && defined(__APPLE__)
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#endif

/** Timer (Linux). */
struct osal_timer {
#if defined(__MACH__) && defined(__APPLE__)
	/* Time base info */
	mach_timebase_info_data_t tb;
	/* Period */
	uint64_t period;
	/* Current target */
	uint64_t target;
#elif defined(__linux__)
	/** Timer file descriptor. */
	int t;
#endif
};

#endif
