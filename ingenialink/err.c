/*
 * MIT License
 *
 * Copyright (c) 2017 Ingenia-CAT S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

void ilerr__set(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(err_last, sizeof(err_last), fmt, args);
	va_end(args);
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
