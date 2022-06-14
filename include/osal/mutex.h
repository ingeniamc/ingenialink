#ifndef OSAL_MUTEX_H_
#define OSAL_MUTEX_H_

/** Mutex. */
typedef struct osal_mutex osal_mutex_t;

/**
 * Create a mutex.
 *
 * @return
 *      Mutex (NULL if it could not be created).
 *
 * @see
 *      osal_mutex_destroy
 */
osal_mutex_t *osal_mutex_create(void);

/**
 * Destroy a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_create
 */
void osal_mutex_destroy(osal_mutex_t *mutex);

/**
 * Acquire a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_unlock
 */
void osal_mutex_lock(osal_mutex_t *mutex);

/**
 * Release a mutex.
 *
 * @param [in] mutex
 *     Valid mutex.
 *
 * @see
 *      osal_mutex_lock
 */
void osal_mutex_unlock(osal_mutex_t *mutex);

#endif
