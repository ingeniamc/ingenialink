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

#ifndef INGENIALINK_FRAME_H_
#define INGENIALINK_FRAME_H_

#include "public/ingenialink/common.h"

/** IngeniaLink frame  minimum size. */
#define IL_FRAME_MIN_SZ		17U
/** IngeniaLink frame  maximum size. */
#define IL_FRAME_MAX_SZ		27U
/** IngeniaLink frame maximum data size. */
#define IL_FRAME_MAX_DATA_SZ	8U

/** Obtain index from frame address. */
#define IL_FRAME_IDX(addr)		((addr) & 0xFFFF)

/** Obtain subindex from frame address. */
#define IL_FRAME_SIDX(addr)		(((addr) >> 16) & 0xFF)

/** Obtain frame address from index and subindex. */
#define IL_FRAME_ADDR(idx, sidx)	(((sidx) << 16) | (idx))

/** IngeniaLink frame state. */
typedef enum {
	/** Waiting for FUNCTION byte */
	IL_FRAME_STATE_WAITING_FUNC,
	/** Waiting for MEI byte */
	IL_FRAME_STATE_WAITING_MEI,
	/** Waiting for DATA SIZE bytes */
	IL_FRAME_STATE_WAITING_DATA,
	/** Waiting for SYNC bytes */
	IL_FRAME_STATE_WAITING_SYNC,
	/** Complete */
	IL_FRAME_STATE_COMPLETE
} il_frame_state_t;

/** IngeniaLink frame. */
typedef struct {
	/** Buffer */
	uint8_t buf[IL_FRAME_MAX_SZ];
	/** Buffer size */
	size_t sz;
	/** State */
	il_frame_state_t state;
} il_frame_t;

/** IngeniaLink frame defaults initializer. */
#define IL_FRAME_INIT_DEF \
	{ { 0U }, 0U, IL_FRAME_STATE_WAITING_FUNC }

/**
 * Initialize a frame.
 *
 * @param [in, out] frame
 *     IngeniaLink frame.
 * @param [in] id
 *     Node ID.
 * @param [in] idx
 *     Index.
 * @param [in] sidx
 *     Subindex.
 * @param [in] data
 *     Data.
 * @param [in] sz
 *     Data size.
 *
 * @returns
 *      IL_EINVAL if the data size if too large.
 */
int il_frame__init(il_frame_t *frame, uint8_t id, uint32_t address,
		   const void *data, size_t sz);

/**
 * Reset frame.
 *
 * @notes
 *      Its size will be set to zero and its state to
 *      IL_FRAME_STATE_WAITING_FUNC.
 *
 * @param [in, out] frame
 *     IngeniaLink frame.
 */
void il_frame__reset(il_frame_t *frame);

/**
 * Push data to a frame and update its state.
 *
 * @note
 *     Once frame is complete, its state will be set to IL_FRAME_STATE_COMPLETE
 *     and the function will return discarding further data.
 *
 * @param [in, out] frame
 *     IngeniaLink frame.
 * @param [in] c
 *      Byte to be pushed.
 *
 * @returns
 *      IL_EFAIL if there is a framing error.
 */
int il_frame__push(il_frame_t *frame, uint8_t c);

/**
 * Obtain frame node ID.
 *
 * @param [in] frame
 *	IngeniaLink frame.
 *
 * @returns
 *	Frame node ID.
 */
uint8_t il_frame__get_id(const il_frame_t *frame);

/**
 * Obtain frame address..
 *
 * @param [in] frame
 *	IngeniaLink frame.
 *
 * @returns
 *	Frame address.
 */
uint32_t il_frame__get_address(const il_frame_t *frame);

/**
 * Obtain frame node data size.
 *
 * @param [in] frame
 *	IngeniaLink frame.
 */
size_t il_frame__get_sz(const il_frame_t *frame);

/**
 * Obtain frame data pointer.
 *
 * @param [in] frame
 *	IngeniaLink frame.
 *
 * @returns
 *	Pointer to the frame data field.
 */
void *il_frame__get_data(il_frame_t *frame);

/**
 * Check if frame is a response.
 *
 * @param [in] frame
 *	IngeniaLink frame.
 *
 * @returns
 *	Non-zero is frame is a response.
 */
int il_frame__is_resp(const il_frame_t *frame);

#endif
