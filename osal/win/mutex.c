#include "mutex.h"

#include <stdlib.h>

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_mutex_t *osal_mutex_create()
{
	osal_mutex_t *mutex;

	mutex = malloc(sizeof(*mutex));
	if (!mutex)
		return NULL;

	InitializeCriticalSection(&mutex->m);

	return mutex;
}

void osal_mutex_destroy(osal_mutex_t *mutex)
{
	DeleteCriticalSection(&mutex->m);
	free(mutex);
}

void osal_mutex_lock(osal_mutex_t *mutex)
{
	EnterCriticalSection(&mutex->m);
}

void osal_mutex_unlock(osal_mutex_t *mutex)
{
	LeaveCriticalSection(&mutex->m);
}
