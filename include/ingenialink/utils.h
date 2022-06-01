#ifndef INGENIALINK_UTILS_H_
#define INGENIALINK_UTILS_H_

#include <stddef.h>

#include "public/ingenialink/common.h"

/** Obtain the minimum of a, b. */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** Obtain the size of an array. */
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

/** Return elements on a circular queue. */
#define CIRC_CNT(head, tail, size) (((head) - (tail)) & ((size) - 1))

/** Return space available on a circular queue. */
#define CIRC_SPACE(head, tail, size) CIRC_CNT((tail), ((head) + 1), (size))

/** Cast a member of a structure out to the containing structure. */
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

/** Swap 16-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_be_16(x) \
	((((uint16_t)(x) & 0xFF00U) >> 8) | \
	 (((uint16_t)(x) & 0x00FFU) << 8))
#else
#define __swap_be_16(x) (x)
#endif

/** Swap 16-bit value on little-endian systems. */
#ifndef IL_BIG_ENDIAN
#define __swap_le_16(x) \
	((((uint16_t)(x) & 0xFF00U) >> 8) | \
	 (((uint16_t)(x) & 0x00FFU) << 8))
#else
#define __swap_le_16(x) (x)
#endif

/** Swap 32-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_be_32(x) \
	((((uint32_t)(x) & 0xFF000000U) >> 24) | \
	 (((uint32_t)(x) & 0x00FF0000U) >>  8) | \
	 (((uint32_t)(x) & 0x0000FF00U) <<  8) | \
	 (((uint32_t)(x) & 0x000000FFU) << 24))
#else
#define __swap_be_32(x) (x)
#endif

/** Swap 64-bit value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_be_64(x) \
	((((uint64_t)(x) & 0xFF00000000000000U) >> 56) | \
	 (((uint64_t)(x) & 0x00FF000000000000U) >> 40) | \
	 (((uint64_t)(x) & 0x0000FF0000000000U) >> 24) | \
	 (((uint64_t)(x) & 0x000000FF00000000U) >>  8) | \
	 (((uint64_t)(x) & 0x00000000FF000000U) <<  8) | \
	 (((uint64_t)(x) & 0x0000000000FF0000U) << 24) | \
	 (((uint64_t)(x) & 0x000000000000FF00U) << 40) | \
	 (((uint64_t)(x) & 0x00000000000000FFU) << 56))
#else
#define __swap_be_64(x) (x)
#endif

/** Swap float value on big-endian systems. */
#ifdef IL_BIG_ENDIAN
#define __swap_be_float(x) \
	((float)((((uint32_t)(x) & 0xFF000000U) >> 24) | \
		 (((uint32_t)(x) & 0x00FF0000U) >>  8) | \
		 (((uint32_t)(x) & 0x0000FF00U) <<  8) | \
		 (((uint32_t)(x) & 0x000000FFU) << 24)))
#else
#define __swap_be_float(x) (x)
#endif

/*
 * Reference counting
 */

/** Reference counter. */
typedef struct il_utils_refcnt il_utils_refcnt_t;

/** De-allocation callback. */
typedef void (*il_utils_refcnt_destroy_t)(void *ctx);

/**
 * Create a reference counter.
 *
 * @param [in] destroy
 *	De-allocation callback.
 * @param [in] ctx
 *	Callback context.
 *
 * @return
 *	Reference counter (NULL if it could not be created).
 */
il_utils_refcnt_t *il_utils__refcnt_create(il_utils_refcnt_destroy_t destroy,
					   void *ctx);

/**
 * Destroy a reference counter.
 *
 * @note
 *	It is automatically called by il_utils__refcnt_release once it hits 0.
 *
 * @param [in]
 *	Reference counter instance.
 */
void il_utils__refcnt_destroy(il_utils_refcnt_t *refcnt);

/**
 * Retain a reference.
 *
 * @param [in]
 *	Reference counter instance.
 */
void il_utils__refcnt_retain(il_utils_refcnt_t *refcnt);

/**
 * Release a reference.
 *
 * @param [in]
 *	Reference counter instance.
 */
void il_utils__refcnt_release(il_utils_refcnt_t *refcnt);

#endif
