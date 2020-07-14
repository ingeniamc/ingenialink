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

#ifndef ETH_NET_H_
#define ETH_NET_H_

#include "../net.h"

#include "ingenialink/utils.h"

#include "osal/osal.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>
#include <winsock2.h>


/** Number of retries while waiting to receive a frame. */
#define NUMBER_OP_RETRIES	3
/** Default read timeout. */
#define READ_TIMEOUT_DEF	10000

/** Vendor ID register address. */
#define VENDOR_ID_ADDR		0x06E0

/** Statusword address. */
#define STATUSWORD_ADDRESS	0x0011

/** ETH network. */
typedef struct il_eth_net {
	/** Network (parent). */
	il_net_t net;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
	/** Slave Address */
    const char *address_ip;
    /** Port */
    int port;
	/** Port IP*/
    int port_ip;
	/** Server: WSAStartup() */
	WSADATA *WSAData;
	/** Socket */
	SOCKET *server;
	/** Socket address */
	SOCKADDR_IN addr;
    /** Stop reconnect */
    int stop_reconnect;
	/** Listener thread. */
	osal_thread_t *listener;
	/** Listener stop flag. */
	int stop;
	/** Protocol. */
	int protocol;

} il_eth_net_t;

/** ETH network device monitor */
typedef struct il_eth_net_dev_mon {
	/** Network monitor (parent). */
	il_net_dev_mon_t mon;
	/** Serial port monitor. */
	ser_dev_mon_t *smon;
	/** Running flag. */
	int running;
	/** Callback */
	il_net_dev_on_evt_t on_evt;
	/** Context */
	void *ctx;
} il_eth_net_dev_mon_t;

/** Obtain ETH Network from parent. */
#define to_eth_net(ptr) container_of(ptr, struct il_eth_net, net)

/** Obtain ETH Network device monitor from parent. */
#define to_eth_mon(ptr) container_of(ptr, struct il_eth_net_dev_mon, mon)

#endif