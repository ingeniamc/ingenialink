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

#ifndef INCLUDE_INGENIALINK_COMMON_H_
#define INCLUDE_INGENIALINK_COMMON_H_

#include "config.h"

#include <stdlib.h>

/*
 * Visual Studio <= 2008 versions do not provide C99 fixed-size integers.
 * Define _IL_NO_LEGACY_STDINT to define them in your project.
 */
#if defined(_MSC_VER)
#  if _MSC_VER > 1500
#    include <stdint.h>
#  elif !defined(_IL_NO_LEGACY_STDINT)
#    if _MSC_VER <= 1500

/*
 * Signed/Unsigned integer limits.
 */
#define INT8_MIN		(-128)
#define INT16_MIN		(-32767 - 1)
#define INT32_MIN		(-2147483647 - 1)
#define INT64_MIN		(9223372036854775807 - 1)

#define INT8_MAX		(127)
#define INT16_MAX		(32767)
#define INT32_MAX		(2147483647)
#define INT64_MAX		(9223372036854775807)

#define UINT8_MAX		(255)
#define UINT16_MAX		(65535)
#define UINT32_MAX		(4294967295U)
#define UINT64_MAX		(18446744073709551615)

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#    else
#      error Unsupported MSC version
#    endif
#  endif
#else
#  include <stdint.h>
#endif

/**
 * @file ingenialink/common.h
 * @brief Common.
 * @ingroup IL
 * @{
 */

#ifdef __cplusplus
/** Start declaration in C++ mode. */
# define IL_BEGIN_DECL extern "C" {
/** End declaration in C++ mode. */
# define IL_END_DECL }
#else
/** Start declaration in C mode. */
# define IL_BEGIN_DECL
/** End declaration in C mode. */
# define IL_END_DECL
#endif

/** Declare a public function exported for application use. */
#ifndef IL_STATIC
# if defined(IL_BUILDING) && __GNUC__ >= 4
#  define IL_EXPORT __attribute__((visibility("default")))
# elif defined(IL_BUILDING) && defined(_MSC_VER)
#  define IL_EXPORT __declspec(dllexport)
# elif defined(_MSC_VER)
#  define IL_EXPORT __declspec(dllimport)
# else
#  define IL_EXPORT
# endif
#else
# define IL_EXPORT
#endif

/** @} */

#endif
