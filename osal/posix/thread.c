#include "thread.h"

#include <stdlib.h>
#include <string.h>

#include "osal/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Thread wrapper.
 *
 * @param [in] args
 *      Arguments (osal_thread_t *).
 *
 * @return
 *      NULL.
 */
static void *thread_wrapper(void *args)
{
	osal_thread_t *thread = args;

	thread->result = thread->func(thread->args);
	thread->isFinished = 1;

	return NULL;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_thread_t *osal_thread_create_(osal_thread_func_t func, void *args)
{
	int r;
	osal_thread_t *thread;

	thread = malloc(sizeof(*thread));
	if (!thread)
		return NULL;

	thread->func = func;
	thread->args = args;
	thread->result = 0;
	thread->isFinished = 0;

	r = pthread_create(&thread->t, NULL, thread_wrapper, thread);
	if (r)
		goto cleanup_thread;

	return thread;

cleanup_thread:
	free(thread);

	return NULL;
}

void osal_thread_join(osal_thread_t *thread, int *result)
{
	if (thread->isFinished != 1) {
		(void)pthread_join(thread->t, NULL);
	}

	if (result)
		*result = thread->result;

	free(thread);
}
