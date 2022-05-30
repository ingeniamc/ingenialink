#ifndef MONITOR_H_
#define MONITOR_H_

#include "public/ingenialink/monitor.h"

#include "osal/osal.h"

/** Monitoring base period (uS). */
#define BASE_PERIOD		100

/** Monitor mapping, index offset. */
#define MAPPING_IDX_OFFSET	16
/** Monitor mapping, sub-index offset. */
#define MAPPING_SIDX_OFFSET	8

/** Monitor trigger source, sub-index offset. */
#define TRIGSRC_SIDX_OFFSET	16

/** Availability wait time (ms) */
#define AVAILABLE_WAIT_TIME	100

#define DEFAULT_SUBNODE_VALUE 0

/** Acquisition context. */
typedef struct {
	/** Acquisition (uses double buffering mechanism). */
	il_monitor_acq_t acq[2];
	/** Current acquisition. */
	int curr;
	/** Size. */
	size_t sz;
	/** Sampling period (s). */
	double t_s;
	/** Maximum number of samples. */
	size_t max_samples;
	/** Lock. */
	osal_mutex_t *lock;
	/** Finished condition. */
	osal_cond_t *finished_cond;
	/** Finished flag. */
	int finished;
	/** Thread. */
	osal_thread_t *td;
	/** Stop flag. */
	int stop;
} il_monitor_acq_ctx_t;

/** IngeniaLink monitor. */
struct il_monitor {
	/** Associated servo. */
	il_servo_t *servo;
	/** Mapped registers. */
	il_reg_t mappings[IL_MONITOR_CH_NUM];
	/** Mappings valid. */
	int mappings_valid[IL_MONITOR_CH_NUM];
	/** Acquisition context. */
	il_monitor_acq_ctx_t acq;
};

#endif
