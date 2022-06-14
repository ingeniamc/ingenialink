#ifndef PUBLIC_INGENIALINK_MONITOR_H_
#define PUBLIC_INGENIALINK_MONITOR_H_

#include "servo.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/monitor.h
 * @brief Monitor.
 * @defgroup IL_MONITOR Monitor
 * @ingroup IL
 * @{
 */

/*
 * References:
 *	http://doc.ingeniamc.com/display/EMCL/Monitoring
 */

/** IngeniaLink monitor. */
typedef struct il_monitor il_monitor_t;

/** Number of channels. */
#define IL_MONITOR_CH_NUM	4

/** Trigger modes. */
typedef enum {
	/** Immediate. */
	IL_MONITOR_TRIGGER_IMMEDIATE = 0,
	/** Motion start. */
	IL_MONITOR_TRIGGER_MOTION = 1,
	/** Positive slope. */
	IL_MONITOR_TRIGGER_POS = 2,
	/** Negative slope. */
	IL_MONITOR_TRIGGER_NEG = 3,
	/** Exit window. */
	IL_MONITOR_TRIGGER_WINDOW = 4,
	/** Digital input activation. */
	IL_MONITOR_TRIGGER_DIN = 5
} il_monitor_trigger_t;

/** Monitor acquisition results.
 *
 * @note
 *	On successful acquisitions, sz and n_samples should be equal. In case of
 *	timeout, n_samples may be less than sz.
 */
typedef struct {
	/** Time buffer. */
	double *t;
	/** Data samples buffers. */
	double *d[IL_MONITOR_CH_NUM];
	/** Size of samples buffer. */
	size_t sz;
	/** Number of actual samples. */
	size_t cnt;
} il_monitor_acq_t;

/**
 * Create monitor instance.
 *
 * @param [in] servo
 *	Associated servo.
 *
 * @return
 *	Monitor instance (NULL if it could not be created).
 */
IL_EXPORT il_monitor_t *il_monitor_create(il_servo_t *servo);

/**
 * Destroy a monitor instance.
 *
 * @param [in] monitor
 *	Monitor instance.
 */
IL_EXPORT void il_monitor_destroy(il_monitor_t *monitor);

/**
 * Start monitor.
 *
 * @param [in] monitor
 *	Monitor instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_start(il_monitor_t *monitor);

/**
 * Stop monitor.
 *
 * @param [in] monitor
 *	Monitor instance.
 */
IL_EXPORT void il_monitor_stop(il_monitor_t *monitor);

/**
 * Wait until current acquisition is completed.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_wait(il_monitor_t *monitor, int timeout);

/**
 * Obtain current available data.
 *
 * @note
 *	The obtained acquisition data can be used until the next call to this
 *	function.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [out] acq
 *	Where the acquisition results will be left.
 */
IL_EXPORT void il_monitor_data_get(il_monitor_t *monitor,
				   il_monitor_acq_t **acq);

/**
 * Configure monitor parameters.
 *
 * @note
 *	- The monitor maximum resolution is 100 us.
 *	- The monitor maximum number of samples depends on the configured
 *	  mappings.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [in] t_s
 *	Sampling period (us).
 * @param [in] delay_samples
 *	Delay samples.
 * @param [in] max_samples
 *	Maximum number of samples to acquire (use 0 to use the maximum).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_configure(il_monitor_t *monitor, unsigned int t_s,
				   size_t delay_samples, size_t max_samples);

/**
 * Configure channel mapping.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [in] ch
 *	Channel to be configured (0-3).
 * @param [in] reg
 *	Register (pre-defined) to be mapped on the channel.
 * @param [in] id
 *	Register ID to be mapped on the channel.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_ch_configure(il_monitor_t *monitor, int ch,
				      const il_reg_t *reg, const char *id);

/**
 * Disable one channel.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [in] ch
 *	Channel to be disabled (0-3).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_ch_disable(il_monitor_t *monitor, int ch);

/**
 * Disable all channels.
 *
 * @param [in] monitor
 *	Monitor instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_ch_disable_all(il_monitor_t *monitor);

/**
 * Trigger configuration.
 *
 * @param [in] monitor
 *	Monitor instance.
 * @param [in] mode
 *	Trigger mode.
 * @param [in] delay_samples
 *	Trigger delay in samples.
 * @param [in] source
 *	Trigger source (only required if mode is IL_MONITOR_TRIGGER_POS,
 *	IL_MONITOR_TRIGGER_NEG or IL_MONITOR_TRIGGER_WINDOW).
 * @param [in] source_id
 *	Trigger source (ID).
 * @param [in] th_pos
 *	Positive threshold (only required if mode is IL_MONITOR_TRIGGER_POS or
 *	IL_MONITOR_TRIGGER_WINDOW).
 * @param [in] th_neg
 *	Negative threshold (only required if mode is IL_MONITOR_TRIGGER_NEG or
 *	IL_MONITOR_TRIGGER_WINDOW).
 * @param [in] din_msk
 *	Digital inputs mask (only required if mode is DIN).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_monitor_trigger_configure(il_monitor_t *monitor,
					   il_monitor_trigger_t mode,
					   size_t delay_samples,
					   const il_reg_t *source,
					   const char *source_id,
					   double th_pos, double th_neg,
					   uint32_t din_msk);

/** @} */

IL_END_DECL

#endif

