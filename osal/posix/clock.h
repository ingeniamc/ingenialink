#ifndef OSAL_POSIX_CLOCK_H_
#define OSAL_POSIX_CLOCK_H_

#include "osal/clock.h"

#if defined(__MACH__) && defined(__APPLE__)
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#elif defined(__linux__)
#  include <time.h>
#endif

/** Performance counter (POSIX) */
struct osal_clock_perf {
#if defined(__MACH__) && defined(__APPLE__)
	/** Timebase info. */
	mach_timebase_info_data_t tb;
	/** Reference time. */
	uint64_t started;
#elif defined(__linux__)
	/** Reference time. */
	struct timespec started;
#endif
};

#endif
