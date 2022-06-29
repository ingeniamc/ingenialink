#include "timer.h"

#include "osal/err.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

osal_timer_t *osal_timer_create()
{
	osal_timer_t *timer;

	timer = malloc(sizeof(*timer));
	if (!timer)
		return NULL;

	timer->hnd = CreateWaitableTimer(NULL, FALSE, NULL);
	if (!timer->hnd)
		goto cleanup_timer;

	return timer;

cleanup_timer:
	free(timer);

	return NULL;
}

void osal_timer_destroy(osal_timer_t *timer)
{
	CancelWaitableTimer(timer->hnd);
	CloseHandle(timer->hnd);
	free(timer);
}

int osal_timer_set(osal_timer_t *timer, osal_time_t period)
{
	LARGE_INTEGER period_;

	period_.QuadPart = -(LONGLONG)period / STEP_SIZE;

	if (!SetWaitableTimer(timer->hnd, &period_,
			      (LONG)period / OSAL_TIMER_NANOSPERMSEC,
			      NULL, NULL, FALSE))
		return OSAL_EFAIL;

	return 0;
}

int osal_timer_wait(osal_timer_t *timer)
{
	if (WaitForSingleObject(timer->hnd, INFINITE) != WAIT_OBJECT_0) {
		if (GetLastError() == ERROR_TIMEOUT)
			return OSAL_ETIMEDOUT;

		return OSAL_EFAIL;
	}

	return 0;
}

