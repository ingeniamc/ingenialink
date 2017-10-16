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

#ifndef NET_H_
#define NET_H_

#include "ingenialink/frame.h"
#include "ingenialink/net.h"
#include "ingenialink/utils.h"

#include "osal/osal.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>

/** Default baudrate. */
#define BAUDRATE_DEF		115200

/** Default read timeout (ms) */
#define TIMEOUT_RD_DEF		100

/** Default write timeout (ms) */
#define TIMEOUT_WR_DEF		100

/** Binary mode ON message (ASCII protocol). */
#define MSG_A2B			"\r0 W 0x82000 1\r"

/** Node scanner timeout (ms) */
#define SCAN_TIMEOUT		100

/** UART node id (index) */
#define UARTCFG_ID_IDX		0x2000

/** UART node id (subindex) */
#define UARTCFG_ID_SIDX		0x01

/** UART configuration, binary mode (index). */
#define UARTCFG_BIN_IDX		0x2000

/** UART configuration, binary mode (subindex). */
#define UARTCFG_BIN_SIDX	0x08

/** Number of binary messages to flush. */
#define BIN_FLUSH		2

/** Statusword (index). */
#define STATUSWORD_IDX		0x6041

/** Statusword (subindex). */
#define STATUSWORD_SIDX		0x00

/** Emergency (index). */
#define EMCY_IDX		0x1003

/** Emergency (subindex). */
#define EMCY_SIDX		0x01

/** Initialization wait time (ms). */
#define INIT_WAIT_TIME		500

/** Statusword subscribers default array size. */
#define SW_SUBS_SZ_DEF		10

/** Emergency subscribers default array size. */
#define EMCY_SUBS_SZ_DEF	10

/** IngeniaLink synchronous transfer context. */
typedef struct {
	/** Node ID. */
	uint8_t id;
	/** Index. */
	uint16_t idx;
	/** Subindex. */
	uint8_t sidx;
	/** Buffer. */
	void *buf;
	/** Buffer size. */
	size_t sz;
	/** Received bytes. */
	size_t *recvd;
	/** Completed flag. */
	int complete;
	/** Lock. */
	osal_mutex_t *lock;
	/** Completed condition variable. */
	osal_cond_t *cond;
} il_net_sync_t;

/** IngeniaLink statusword update subscriber. */
struct il_net_sw_subscriber {
	/** Node ID. */
	uint8_t id;
	/** Callback. */
	il_net_sw_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
};

/**
 * IngeniaLink statusword update subscribers.
 *
 * @note
 *	This is implemented using a dynamic array so that traverse is more
 *	efficient.
 */
typedef struct {
	/** Array of subscribers. */
	il_net_sw_subscriber_t *subs;
	/** Array size. */
	int sz;
	/** Lock. */
	osal_mutex_t *lock;
} il_net_sw_subscriber_lst_t;

/** IngeniaLink emergency subscriber. */
struct il_net_emcy_subscriber {
	/** Node ID. */
	uint8_t id;
	/** Callback. */
	il_net_emcy_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
};

/**
 * IngeniaLink emergency subscribers.
 *
 * @note
 *	This is implemented using a dynamic array so that traverse is more
 *	efficient.
 */
typedef struct {
	/** Array of subscribers. */
	il_net_emcy_subscriber_t *subs;
	/** Array size. */
	int sz;
	/** Lock. */
	osal_mutex_t *lock;
} il_net_emcy_subscriber_lst_t;

/** IngeniaLink network. */
struct il_net {
	/** Serial communications channel. */
	ser_t *ser;
	/** Reference counter. */
	refcnt_t *refcnt;
	/** Network state. */
	il_net_state_t state;
	/** Network state lock. */
	osal_mutex_t *state_lock;
	/** Listener thread. */
	osal_thread_t *listener;
	/** Listener stop flag. */
	int stop;
	/** Network lock. */
	osal_mutex_t *lock;
	/** Network synchronous transfers context. */
	il_net_sync_t sync;
	/** Statusword updates subcribers. */
	il_net_sw_subscriber_lst_t sw_subs;
	/** Emergency subcribers. */
	il_net_emcy_subscriber_lst_t emcy_subs;
};

/** IngeniaLink network device monitor */
struct il_net_dev_mon {
	/** Serial port monitor. */
	ser_dev_mon_t *mon;
	/** Running flag. */
	int running;
	/** Callback */
	il_net_dev_on_evt_t on_evt;
	/** Context */
	void *ctx;
};

#endif
