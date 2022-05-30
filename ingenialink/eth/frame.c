#include "frame.h"

#include "ingenialink/err.h"
#include "ingenialink/utils.h"

#include <string.h>

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_eth_mcb_frame__swap(uint8_t *frame, size_t sz)
{
	size_t i;

	for (i = 0; i < sz; i += 2) {
		uint8_t tmp;

		tmp = frame[i];
		frame[i] = frame[i + 1];
		frame[i + 1] = tmp;
	}
}

void il_eth_mcb_frame__get_address(uint16_t *frame) 
{
	// uint16_t addr;

	// memcpy(&addr, &frame->buf[FR_INDEX_H_FLD], sizeof(idx));
	// idx = __swap_index(idx);
	// sidx = frame->buf[FR_SINDEX_FLD];

	// return IL_EUSB_FRAME_ADDR(idx, sidx);
}
