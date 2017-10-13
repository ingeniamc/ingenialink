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

#ifndef INGENIALINK_NET_H_
#define INGENIALINK_NET_H_

#include "public/ingenialink/net.h"

/** Statusword updates subcriber. */
typedef struct il_net_sw_subscriber il_net_sw_subscriber_t;

/** Statusword updates subcriber callback. */
typedef void (*il_net_sw_subscriber_cb_t)(void *ctx, uint16_t sw);

/** Emergency subcriber. */
typedef struct il_net_emcy_subscriber il_net_emcy_subscriber_t;

/** Emergency subcriber callback. */
typedef void (*il_net_emcy_subscriber_cb_t)(void *ctx, uint32_t code);

/**
 * Retain a reference of the network.
 *
 * @param [in] net
 *	IngeniaLink network.
 */
void il_net__retain(il_net_t *net);

/**
 * Release a reference of the network.
 *
 * @param [in] net
 *	IngeniaLink network.
 */
void il_net__release(il_net_t *net);

/**
 * Write.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Node id.
 * @param [in] idx
 *	Index.
 * @param [in] sidx
 *	Subindex.
 * @param [in] buf
 *	Data buffer (optional).
 * @param [in] sz
 *	Data buffer size.
 * @param [in] confirmed
 *	Flag to confirm the write.
 * @param [in] timeout
 *	Confirmation timeout (ms).
 *
 * @returns
 *	0 on success, error code otherwise.
 */
int il_net__write(il_net_t *net, uint8_t id, uint16_t idx, uint8_t sidx,
		  const void *buf, size_t sz, int confirmed, int timeout);

/**
 * Read.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Expected node id (0 to match any).
 * @param [in] idx
 *	Expected index.
 * @param [in] sidx
 *	Expected subindex.
 * @param [out] buf
 *	Data output buffer.
 * @param [in] sz
 *	Data buffer size.
 * @param [out] recvd
 *	Actual number of received data bytes (optional).
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @returns
 *	0 on success, error code otherwise.
 */
int il_net__read(il_net_t *net, uint8_t id, uint16_t idx, uint8_t sidx,
		 void *buf, size_t sz, size_t *recvd, int timeout);

/**
 * Subscribe to statusword updates.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Node ID.
 * @param [in] cb
 *	Callback.
 * @param [in] ctx
 *	Callback context.
 *
 * @returns
 *	Assigned slot (>=0) or error code (< 0).
 */
int il_net__sw_subscribe(il_net_t *net, uint8_t id,
			 il_net_sw_subscriber_cb_t cb, void *ctx);

/**
 * Unubscribe to statusword updates.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] slot
 *	Slot assigned when subscribed.
 */
void il_net__sw_unsubscribe(il_net_t *net, int slot);

/**
 * Subscribe to emergency messages.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Node ID.
 * @param [in] cb
 *	Callback.
 * @param [in] ctx
 *	Callback context.
 *
 * @returns
 *	Assigned slot (>=0) or error code (< 0).
 */
int il_net__emcy_subscribe(il_net_t *net, uint8_t id,
			   il_net_emcy_subscriber_cb_t cb, void *ctx);

/**
 * Unubscribe from emergency messages.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] slot
 *	Slot assigned when subscribed.
 */
void il_net__emcy_unsubscribe(il_net_t *net, int slot);

#endif
