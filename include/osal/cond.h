#ifndef OSAL_COND_H_
#define OSAL_COND_H_

#include "mutex.h"

/** Condition variable. */
typedef struct osal_cond osal_cond_t;

/**
 * Create a condition variable.
 *
 * @return
 *	Condition variable (NULL if it could not be created).
 *
 * @see
 *	osal_cond_destroy
 */
osal_cond_t *osal_cond_create(void);

/**
 * Destroy a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 *
 * @see
 *	osal_cond_init
 */
void osal_cond_destroy(osal_cond_t *cond);

/**
 * Wait for a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 * @param [in] mutex
 *	Valid mutex, used to enter the critical section.
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @return
 *      0 on successful acquisition, error code otherwise.
 */
int osal_cond_wait(osal_cond_t *cond, osal_mutex_t *mutex, int timeout);

/**
 * Signal a condition variable.
 *
 * @param [in] cond
 *	Valid condition variable.
 */
void osal_cond_signal(osal_cond_t *cond);

/**
 * Signal all condition variables.
 *
 * @param [in] cond
 *	Valid condition variable.
 */
void osal_cond_broadcast(osal_cond_t *cond);

#endif
