#include "mutex.h"

#include <stdlib.h>

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_mutex_t *osal_mutex_create()
{
	osal_mutex_t *mutex;
	int r;

	mutex = malloc(sizeof(*mutex));
	if (!mutex)
		return NULL;

	r = pthread_mutex_init(&mutex->m, NULL);
	if (r)
		goto cleanup_mutex;

	return mutex;

cleanup_mutex:
	free(mutex);
	return NULL;
}

void osal_mutex_destroy(osal_mutex_t *mutex)
{
	(void)pthread_mutex_destroy(&mutex->m);
	free(mutex);
}

void osal_mutex_lock(osal_mutex_t *mutex)
{
	(void)pthread_mutex_lock(&mutex->m);
}

void osal_mutex_unlock(osal_mutex_t *mutex)
{
	(void)pthread_mutex_unlock(&mutex->m);
}
