#ifndef POLLER_H_
#define POLLER_H_

#include "public/ingenialink/poller.h"

#include "osal/osal.h"


#define DEFAULT_SUBNODE_VALUE 0

/** IngeniaLink register poller. */
struct il_poller {
	/** Associated servo. */
	il_servo_t *servo;
	/** Number of channels. */
	size_t n_ch;
	/** Mapped registers to each channel. */
	il_reg_t *mappings;
	/** Mappings validity. */
	int *mappings_valid;
	/** Acquisition (uses double buffering mechanism). */
	il_poller_acq_t acq[2];
	/** Current acquisition. */
	int acq_curr;
	/** Sampling period (ms). */
	int t_s;
	/** Buffer size. */
	size_t sz;
	/** Timer. */
	osal_timer_t *timer;
	/** Performance counter. */
	osal_clock_perf_t *perf;
	/** Lock. */
	osal_mutex_t *lock;
	/** Thread. */
	osal_thread_t *td;
	/** Running flag. */
	int running;
	/** Stop flag. */
	int stop;
};

#endif
