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

#include "ingenialink/net.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>

/** Default baudrate. */
#define BAUDRATE_DEF		115200

/** Default write timeout (ms) */
#define TIMEOUT_WR_DEF		1000

/** Binary mode ON message (ASCII protocol). */
#define MSG_A2B			"\r0 W 0x82000 1\r"

/** UART node id (index) */
#define UARTCFG_ID_IDX		0x2000

/** UART node id (subindex) */
#define UARTCFG_ID_SIDX		0x01

/** UART configuration, binary mode (index). */
#define UARTCFG_BIN_IDX		0x2000
/** UART configuration, binary mode (subindex). */
#define UARTCFG_BIN_SIDX	0x08

/** Monitor wait time (ms). */
#define MONITOR_WAIT_TIME	2000

/** IngeniaLink network. */
struct il_net {
	/** Serial communications channel */
	ser_t *ser;
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
