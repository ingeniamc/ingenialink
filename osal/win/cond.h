#ifndef OSAL_WIN_COND_H_
#define OSAL_WIN_COND_H_

#include "osal/cond.h"

#include <Windows.h>

/** Condition variable (Windows). */
struct osal_cond {
	/** Condition variable. */
	CONDITION_VARIABLE c;
};

#endif
