/*
 * MIT License
 *
 * Copyright (c) 2017-2018 Ingenia-CAT S.L.
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

#ifndef EUSB_NET_H_
#define EUSB_NET_H_

#include "../net.h"

#include "ingenialink/eusb/frame.h"
#include "ingenialink/utils.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>

/** Default baudrate. */
#define BAUDRATE_DEF		115200

/** Serial port read poll timeout (ms). */
#define SER_POLL_TIMEOUT	100

/** Binary mode ON message (ASCII protocol). */
#define MSG_A2B			"\r0 W 0x82000 1\r"

/** Node scanner timeout (ms) */
#define SCAN_TIMEOUT		1000

/** UART node id (index) */
#define UARTCFG_ID_ADDRESS	0x012000

/** UART configuration, binary mode (index). */
#define UARTCFG_BIN_ADDRESS	0x082000

/** Number of binary messages to flush. */
#define BIN_FLUSH		2

/** Statusword address. */
#define STATUSWORD_ADDRESS	0x006041

/** Emergency address. */
#define EMCY_ADDRESS		0x011003

/** Initialization wait time (ms). */
#define INIT_WAIT_TIME		500

/** Synchronous transfers context. */
typedef struct {
	/** Node ID. */
	uint8_t id;
	/** Address. */
	uint32_t address;
	/** Buffer. */
	void *buf;
	/** Buffer size. */
	size_t sz;
	/** Completed flag. */
	int complete;
	/** Lock. */
	osal_mutex_t *lock;
	/** Completed condition variable. */
	osal_cond_t *cond;
} il_eusb_net_sync_t;

/** E-USB Network. */
typedef struct il_eusb_net {
	/** Network (parent) */
	il_net_t net;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
	/** Serial communications channel. */
	ser_t *ser;
	/** Serial communications options. */
	ser_opts_t sopts;
	/** Listener thread. */
	osal_thread_t *listener;
	/** Listener stop flag. */
	int stop;
	/** Synchronous transfers context. */
	il_eusb_net_sync_t sync;
} il_eusb_net_t;

/** E-USB Network device monitor */
typedef struct il_eusb_net_dev_mon {
	/** Network device monitor (parent). */
	il_net_dev_mon_t mon;
	/** Serial port monitor. */
	ser_dev_mon_t *smon;
	/** Running flag. */
	int running;
	/** Callback */
	il_net_dev_on_evt_t on_evt;
	/** Context */
	void *ctx;
} il_eusb_net_dev_mon_t;

/** Obtain E-USB Network from parent. */
#define to_eusb_net(ptr) container_of(ptr, struct il_eusb_net, net)

/** Obtain E-USB Network device monitor from parent. */
#define to_eusb_mon(ptr) container_of(ptr, struct il_eusb_net_dev_mon, mon)

#endif
