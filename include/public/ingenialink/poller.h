#ifndef PUBLIC_INGENIALINK_POLLER_H_
#define PUBLIC_INGENIALINK_POLLER_H_

#include "servo.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/poller.h
 * @brief Poller.
 * @defgroup IL_POLLER Poller
 * @ingroup IL
 * @{
 */

/** IngeniaLink poller. */
typedef struct il_poller il_poller_t;

/** Poller acquisition results. */
typedef struct {
	/** Time vector. */
	double *t;
	/** Data vectors. */
	double **d;
	/** Number of actual samples in the time and data vectors. */
	size_t cnt;
	/** Data lost flag. */
	int lost;
} il_poller_acq_t;

/**
 * Create a register poller.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] n_ch
 *	Number of channels.
 *
 * @return
 *	Poller instance (NULL if it could not be created).
 */
IL_EXPORT il_poller_t *il_poller_create(il_servo_t *servo, size_t n_ch);

/**
 * Destroy a register poller.
 *
 * @param [in] poller
 *	Poller instance.
 */
IL_EXPORT void il_poller_destroy(il_poller_t *poller);

/**
 * Start poller.
 *
 * @param [in] poller
 *	Poller instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_start(il_poller_t *poller);

/**
 * Stop poller.
 *
 * @param [in] poller
 *	Poller instance.
 */
IL_EXPORT void il_poller_stop(il_poller_t *poller);

/**
 * Obtain current time and data vectors.
 *
 * @note
 *	The obtained acquisition data can be used until the next call to this
 *	function.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [out] acq
 *	Where the acquisition results will be left.
 */
IL_EXPORT void il_poller_data_get(il_poller_t *poller, il_poller_acq_t **acq);

/**
 * Configure poller parameters.
 *
 * @note
 *	- The maximum stable polling rate is ~500 Hz (2 ms) for a single
 *	  register when the servo is enabled.
 *	- The buffer size must be set according to your application needs. It
 *	  should be large enough so that it can store all samples collected
 *	  between subsequent calls to `il_poller_data_get`.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] t_s
 *	Sampling period (ms).
 * @param [in] buf_sz
 *	Buffer size.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_configure(il_poller_t *poller, unsigned int t_s,
				  size_t buf_sz);

/**
 * Configure a poller channel.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] ch
 *	Channel.
 * @param [in] reg
 *	Register (pre-defined) to be polled on this channel.
 * @param [in] id
 *	Register ID to be polled on this channel.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_configure(il_poller_t *poller, unsigned int ch,
				     const il_reg_t *reg, const char *id);

/**
 * Disable a poller channel.
 *
 * @param [in] poller
 *	Poller instance.
 * @param [in] ch
 *	Channel.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_disable(il_poller_t *poller, unsigned int ch);

/**
 * Disable all poller channels.
 *
 * @param [in] poller
 *	Poller instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_poller_ch_disable_all(il_poller_t *poller);

/** @} */

IL_END_DECL

#endif
