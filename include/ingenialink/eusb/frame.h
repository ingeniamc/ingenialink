#ifndef INGENIALINK_EUSB_FRAME_H_
#define INGENIALINK_EUSB_FRAME_H_

#include "public/ingenialink/common.h"

/** IngeniaLink frame  minimum size. */
#define IL_EUSB_FRAME_MIN_SZ		17U
/** IngeniaLink frame  maximum size. */
#define IL_EUSB_FRAME_MAX_SZ		27U
/** IngeniaLink frame maximum data size. */
#define IL_EUSB_FRAME_MAX_DATA_SZ	8U

/** Obtain index from frame address. */
#define IL_EUSB_FRAME_IDX(addr)		((addr) & 0xFFFF)

/** Obtain subindex from frame address. */
#define IL_EUSB_FRAME_SIDX(addr)	(((addr) >> 16) & 0xFF)

/** Obtain frame address from index and subindex. */
#define IL_EUSB_FRAME_ADDR(idx, sidx)	(((sidx) << 16) | (idx))

/** E-USB frame state. */
typedef enum {
	/** Waiting for FUNCTION byte */
	IL_EUSB_FRAME_STATE_WAITING_FUNC,
	/** Waiting for MEI byte */
	IL_EUSB_FRAME_STATE_WAITING_MEI,
	/** Waiting for DATA SIZE bytes */
	IL_EUSB_FRAME_STATE_WAITING_DATA,
	/** Waiting for SYNC bytes */
	IL_EUSB_FRAME_STATE_WAITING_SYNC,
	/** Complete */
	IL_EUSB_FRAME_STATE_COMPLETE
} il_eusb_frame_state_t;

/** E-USB frame. */
typedef struct {
	/** Buffer */
	uint8_t buf[IL_EUSB_FRAME_MAX_SZ];
	/** Buffer size */
	size_t sz;
	/** State */
	il_eusb_frame_state_t state;
} il_eusb_frame_t;

/** IngeniaLink frame defaults initializer. */
#define IL_EUSB_FRAME_INIT_DEF \
	{ { 0U }, 0U, IL_EUSB_FRAME_STATE_WAITING_FUNC }

/**
 * Initialize a frame.
 *
 * @param [in, out] frame
 *     Frame.
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
 *      IL_EINVAL if the data size is too large.
 */
int il_eusb_frame__init(il_eusb_frame_t *frame, uint8_t id, uint32_t address,
			const void *data, size_t sz);

/**
 * Reset frame.
 *
 * @notes
 *      Its size will be set to zero and its state to
 *      IL_EUSB_FRAME_STATE_WAITING_FUNC.
 *
 * @param [in, out] frame
 *     IngeniaLink frame.
 */
void il_eusb_frame__reset(il_eusb_frame_t *frame);

/**
 * Push data to a frame and update its state.
 *
 * @note
 *     Once frame is complete, its state will be set to
 *     IL_EUSB_FRAME_STATE_COMPLETE and the function will return discarding
 *     further data.
 *
 * @param [in, out] frame
 *	Frame.
 * @param [in] c
 *      Byte to be pushed.
 *
 * @returns
 *      IL_EFAIL if there is a framing error.
 */
int il_eusb_frame__push(il_eusb_frame_t *frame, uint8_t c);

/**
 * Obtain frame node ID.
 *
 * @param [in] frame
 *	Frame.
 *
 * @returns
 *	Frame node ID.
 */
uint8_t il_eusb_frame__get_id(const il_eusb_frame_t *frame);

/**
 * Obtain frame address.
 *
 * @param [in] frame
 *	Frame.
 *
 * @returns
 *	Frame address.
 */
uint32_t il_eusb_frame__get_address(const il_eusb_frame_t *frame);

/**
 * Obtain frame node data size.
 *
 * @param [in] frame
 *	Frame.
 */
size_t il_eusb_frame__get_sz(const il_eusb_frame_t *frame);

/**
 * Obtain frame data pointer.
 *
 * @param [in] frame
 *	Frame.
 *
 * @returns
 *	Pointer to the frame data field.
 */
void *il_eusb_frame__get_data(il_eusb_frame_t *frame);

/**
 * Check if frame is a response.
 *
 * @param [in] frame
 *	Frame.
 *
 * @returns
 *	Non-zero is frame is a response.
 */
int il_eusb_frame__is_resp(const il_eusb_frame_t *frame);

#endif
