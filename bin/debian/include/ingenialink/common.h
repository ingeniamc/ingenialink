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
#include <stdint.h>

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
