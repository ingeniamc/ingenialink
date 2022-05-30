#include "thread.h"

#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Thread wrapper.
 *
 * @param [in] args
 *      Arguments (osal_thread *).
 *
 * @return
 *      0.
 */
static DWORD WINAPI thread_wrapper(PVOID args)
{
	osal_thread_t *thread = args;

	thread->result = thread->func(thread->args);
	thread->isFinished = 1;

	return 0;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_thread_t *osal_thread_create_(osal_thread_func_t func, void *args)
{
	osal_thread_t *thread;

	thread = malloc(sizeof(*thread));
	if (!thread)
		return NULL;

	thread->func = func;
	thread->args = args;
	thread->result = 0;
	thread->isFinished = 0;

	thread->t = CreateThread(NULL, 0, thread_wrapper, thread, 0, NULL);
	if (!thread->t)
		goto cleanup_thread;

	return thread;

cleanup_thread:
	free(thread);

	return NULL;
}

void osal_thread_join(osal_thread_t *thread, int *result)
{
	if (thread->isFinished != 1) {
		WaitForSingleObject(thread->t, INFINITE);
		CloseHandle(thread->t);
	}

	if (result)
		*result = thread->result;

	free(thread);
}
