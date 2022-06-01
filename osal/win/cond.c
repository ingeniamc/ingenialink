#include "cond.h"

#include <stdlib.h>
#include <string.h>

#include "mutex.h"
#include "osal/err.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_cond_t *osal_cond_create()
{
	osal_cond_t *cond;

	cond = malloc(sizeof(*cond));
	if (!cond)
		return NULL;

	InitializeConditionVariable(&cond->c);

	return cond;
}

void osal_cond_destroy(osal_cond_t *cond)
{
	free(cond);
}

int osal_cond_wait(osal_cond_t *cond, osal_mutex_t *mutex, int timeout)
{
	DWORD timeout_;

	if (timeout <= 0)
		timeout_ = INFINITE;
	else
		timeout_ = timeout;

	if (!SleepConditionVariableCS(&cond->c, &mutex->m, timeout_)) {
		if (GetLastError() == ERROR_TIMEOUT)
			return OSAL_ETIMEDOUT;

		return OSAL_EFAIL;
	}

	return 0;
}

void osal_cond_signal(osal_cond_t *cond)
{
	WakeConditionVariable(&cond->c);
}

void osal_cond_broadcast(osal_cond_t *cond)
{
	WakeAllConditionVariable(&cond->c);
}

