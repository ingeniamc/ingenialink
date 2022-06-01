#include "ingenialink/err.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>

#include <stdarg.h>
#include <stdio.h>

/*******************************************************************************
 * Internal
 ******************************************************************************/

/** Declare a variable thread-local. */
#ifdef __GNUC__
# define thread_local __thread
#elif __STDC_VERSION__ >= 201112L
# define thread_local _Thread_local
#elif defined(_MSC_VER)
# define thread_local __declspec(thread)
#else
# define thread_local
# warning Thread Local Storage (TLS) not available. Last error will be global.
#endif

/** Maximum error message size. */
#define ERR_SZ 256U

/** Global error description. */
static thread_local char err_last[ERR_SZ] = "";

static thread_local int err_ipb_last = 0;

void ilerr__set(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(err_last, sizeof(err_last), fmt, args);
	va_end(args);
}

void ilerr__ipb_set(int err)
{
	err_ipb_last = err;
}

int ilerr__ser(int32_t code)
{
	int r;

	switch (code) {
	case SER_ETIMEDOUT:
		ilerr__set("Operation timed out");
		r = IL_ETIMEDOUT;
		break;
	case SER_EDISCONN:
		ilerr__set("Device was disconnected");
		r = IL_EDISCONN;
		break;
	default:
		ilerr__set("sercomm: %s", sererr_last());
		r = IL_EFAIL;
		break;
	}

	return r;
}

int ilerr__eth(int32_t code)
{
	int r = code;

	switch (code) {
	case IL_EWRONGCRC:
		ilerr__set("Communications error (CRC mismatch)");
		break;
	case IL_ENACK:
		ilerr__set("Communications error (NACK)");
		break;
	case IL_EWRONGREG:
		ilerr__set("Wrong address error");
		break;
	case IL_REGNOTFOUND:
		ilerr__set("Register not found");
		break;
	case IL_ETIMEDOUT:
		ilerr__set("Operation timed out");
		break;
	case IL_EIO:
		ilerr__set("Communications error");
		break;
	case IL_ENOTSUP:
		ilerr__set("Functionality not supported");
		break;
	default:
		ilerr__set("Generic error");
		r = IL_EFAIL;
		break;
	}

	return r;

}

int ilerr__ecat(int32_t code)
{
	int r;
	/** TODO: LWIP & SOEM errors */
	switch (code) {
	case IL_EWRONGCRC:
		ilerr__set("Communications error (CRC mismatch)");
		break;
	case IL_ENACK:
		ilerr__set("Communications error (NACK)");
		break;
	case IL_EWRONGREG:
		ilerr__set("Wrong address error");
		break;
	case IL_REGNOTFOUND:
		ilerr__set("Register not found");
		break;
	case IL_ETIMEDOUT:
		ilerr__set("Operation timed out");
		break;
	case IL_EIO:
		ilerr__set("Communications error");
		break;
	case IL_ENOTSUP:
		ilerr__set("Functionality not supported");
		break;
	default:
		ilerr__set("Generic error");
		r = IL_EFAIL;
		break;
	}

	return r;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

const char *ilerr_last()
{
	return (const char *)err_last;
}

int *ilerr_ipb_last()
{
	return err_ipb_last;
}
