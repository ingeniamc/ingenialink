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
