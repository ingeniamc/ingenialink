#ifndef OSAL_TIMER_H_
#define OSAL_TIMER_H_

/** Nanoseconds per second. */
#define OSAL_TIMER_NANOSPERSEC		1000000000LL

/** Nanoseconds per millisecond. */
#define OSAL_TIMER_NANOSPERMSEC		1000000L

/** Nanoseconds per microsecond. */
#define OSAL_TIMER_NANOSPERUSEC		1000

/** Time value (nanoseconds base). */
typedef long long int osal_time_t;

/** Timer instance. */
typedef struct osal_timer osal_timer_t;

/**
 * Create a timer.
 *
 * @return
 *	Timer instance (NULL if it could not be created).
 */
osal_timer_t *osal_timer_create(void);

/**
 * Destroy a timer.
 *
 * @param [in] timer
 *	Timer instance.
 */
void osal_timer_destroy(osal_timer_t *timer);

/**
 * Set (arm) a timer.
 *
 * @param [in] timer
 *	Timer instance.
 * @param [in] period
 *	Timer period (ns).
 *
 * @return
 *	0 on success, error code otherwise.
 */
int osal_timer_set(osal_timer_t *timer, osal_time_t period);

/**
 * Wait until timer expires.
 *
 * @param [in] timer
 *	Timer instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
int osal_timer_wait(osal_timer_t *timer);

#endif
