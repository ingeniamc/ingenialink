#ifndef OSAL_THREAD_H_
#define OSAL_THREAD_H_

/** Thread. */
typedef struct osal_thread osal_thread_t;

/** Thread function prototype. */
typedef int (*osal_thread_func_t)(void *args);

/**
 * Create a thread.
 *
 * @param [in] func
 *	Thread function.
 * @param [in] args
 *	Arguments passed to the thread.
 *
 * @return
 *	Thread (NULL if it could not be created).
 */
osal_thread_t *osal_thread_create_(osal_thread_func_t func, void *args);

/**
 * Join a thread (and destroy it).
 *
 * @param [in] thread
 *	Valid Thread.
 * @param [out] result
 *	Thread return code (optional).
 */
void osal_thread_join(osal_thread_t *thread, int *result);

#endif
