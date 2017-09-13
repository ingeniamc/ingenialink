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

#include "frame.h"

#include "ingenialink/err.h"
#include "ingenialink/utils.h"

#include <string.h>

/*******************************************************************************
 * Private
 ******************************************************************************/

/** Frame trailing sync block. */
static const uint8_t sync[FR_SYNC_SZ] = { FR_SYNC, FR_SYNC, FR_SYNC, FR_SYNC };

/**
 * Update the frame state.
 *
 * @param [in, out] frame
 *	IngeniaLink frame.
 *
 * @returns
 *	IL_EFAIL if there is any framing error.
 */
static int state_update(il_frame_t *frame)
{
	switch (frame->state) {
	case IL_FRAME_STATE_WAITING_FUNC:
		if (frame->sz == (FR_FUNC_FLD + 1)) {
			if (frame->buf[FR_FUNC_FLD] == FR_FUNC) {
				frame->state = IL_FRAME_STATE_WAITING_MEI;
			} else {
				ilerr__set("Unexpected FUNC code");
				return IL_EFAIL;
			}
		}

		break;
	case IL_FRAME_STATE_WAITING_MEI:
		if (frame->buf[FR_MEI_FLD] == FR_MEI) {
			frame->state = IL_FRAME_STATE_WAITING_DATA;
		} else {
			ilerr__set("Unexpected MEI code");
			return IL_EFAIL;
		}

		break;
	case IL_FRAME_STATE_WAITING_DATA:
		if (frame->sz == (FR_NDATA_L_FLD + 1)) {
			if (frame->buf[FR_NDATA_L_FLD] > IL_FRAME_MAX_DATA_SZ) {
				ilerr__set("Received data size is too large");
				return IL_EFAIL;
			}

			frame->state = IL_FRAME_STATE_WAITING_SYNC;
		}

		break;
	case IL_FRAME_STATE_WAITING_SYNC:
		if (frame->sz == (FR_DATA_FLD + frame->buf[FR_NDATA_L_FLD] +
				  FR_SYNC_SZ)) {
			if (memcmp(&frame->buf[frame->sz - FR_SYNC_SZ], sync,
				   sizeof(sync)) == 0) {
				frame->state = IL_FRAME_STATE_COMPLETE;
			} else {
				ilerr__set("Frame synchronization failed");
				return IL_EFAIL;
			}
		}

		break;
	default:
		break;
	}

	return 0;
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

int il_frame__init(il_frame_t *frame, uint8_t id, uint16_t idx, uint8_t sidx,
		   const void *data, size_t sz)
{
	uint16_t idx_;

	/* validate size */
	if (sz > IL_FRAME_MAX_DATA_SZ) {
		ilerr__set("Data size is too large");
		return IL_EINVAL;
	}

	/* address, fixed fields, etc. */
	frame->buf[FR_ADDR_FLD] = id;
	frame->buf[FR_FUNC_FLD] = FR_FUNC;
	frame->buf[FR_MEI_FLD] = FR_MEI;

	if (data)
		frame->buf[FR_PROT_FLD] = 1;
	else
		frame->buf[FR_PROT_FLD] = 0;

	frame->buf[FR_RES_FLD] = 0;
	frame->buf[FR_NODE_FLD] = id;

	/* index, subindex, address (0) */
	idx_ = __swap_index(idx);
	memcpy(&frame->buf[FR_INDEX_H_FLD], &idx_, sizeof(idx_));
	frame->buf[FR_SINDEX_FLD] = sidx;
	frame->buf[FR_SADDR_H_FLD] = 0;
	frame->buf[FR_SADDR_L_FLD] = 0;

	/* data size, data */
	frame->buf[FR_NDATA_H_FLD] = 0;
	frame->buf[FR_NDATA_L_FLD] = (uint8_t)sz;
	if (data)
		memcpy(&frame->buf[FR_DATA_FLD], data, sz);

	/* trailing sync bytes */
	memcpy(&frame->buf[IL_FRAME_MIN_SZ + sz - FR_SYNC_SZ], sync,
	       FR_SYNC_SZ);

	/* update buffer counter */
	frame->sz = IL_FRAME_MIN_SZ + sz;
	frame->state = IL_FRAME_STATE_COMPLETE;

	return 0;
}

void il_frame__reset(il_frame_t *frame)
{
	frame->state = IL_FRAME_STATE_WAITING_FUNC;
	frame->sz = 0;
}

int il_frame__push(il_frame_t *frame, uint8_t c)
{
	/* check buffer size */
	if (frame->sz >= sizeof(frame->buf)) {
		ilerr__set("Buffer full");
		return IL_EFAIL;
	}

	/* push byte, update state */
	frame->buf[frame->sz] = c;
	frame->sz += 1;

	/* update state, terminate if complete */
	return state_update(frame);
}

uint8_t il_frame__get_id(const il_frame_t *frame)
{
	return frame->buf[FR_ADDR_FLD];
}

uint16_t il_frame__get_idx(const il_frame_t *frame)
{
	uint16_t idx;

	memcpy(&idx, &frame->buf[FR_INDEX_H_FLD], sizeof(idx));
	idx = __swap_index(idx);

	return idx;
}

uint8_t il_frame__get_sidx(const il_frame_t *frame)
{
	return frame->buf[FR_SINDEX_FLD];
}

size_t il_frame__get_sz(const il_frame_t *frame)
{
	return (size_t)frame->buf[FR_NDATA_L_FLD];
}

void *il_frame__get_data(il_frame_t *frame)
{
	return &frame->buf[FR_DATA_FLD];
}

int il_frame__is_resp(const il_frame_t *frame)
{
	return (int)frame->buf[FR_PROT_FLD];
}
