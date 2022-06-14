#ifndef INGENIALINK_MCB_FRAME_H_
#define INGENIALINK_MCB_FRAME_H_

#include "public/ingenialink/common.h"

/**
 * Swap a buffer word-wise on LE systems.
 *
 * @param [in, out] frame
 *	Frame.
 * @param [in] sz
 *	Size.
 */
#ifndef IL_BIG_ENDIAN
void il_mcb_frame__swap(uint8_t *frame, size_t sz);
#else
#define il_mcb_frame__swap(frame, sz)
#endif

#endif
