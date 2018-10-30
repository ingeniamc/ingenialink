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

#ifndef NET_H_
#define NET_H_

#include "ingenialink/net.h"
#include "ingenialink/utils.h"

#include "osal/osal.h"

/** Statusword subscribers default array size. */
#define SW_SUBS_SZ_DEF		10

/** Emergency subscribers default array size. */
#define EMCY_SUBS_SZ_DEF	10

/** Statusword update subscriber. */
struct il_net_sw_subscriber {
	/** Node ID. */
	uint16_t id;
	/** Callback. */
	il_net_sw_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
};

/**
 * Statusword update subscribers.
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

/** Emergency subscriber. */
struct il_net_emcy_subscriber {
	/** Node ID. */
	uint16_t id;
	/** Callback. */
	il_net_emcy_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
};

/**
 * Emergency subscribers.
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

/** Network. */
struct il_net {
	/** Protocol */
	il_net_prot_t prot;
	/** Port */
	char *port;
	/** Read timeout. */
	int timeout_rd;
	/** Write timeout. */
	int timeout_wr;
	/** Network lock. */
	osal_mutex_t *lock;
	/** Network state. */
	il_net_state_t state;
	/** Network state lock. */
	osal_mutex_t *state_lock;
	/** Status updates subcribers. */
	il_net_sw_subscriber_lst_t sw_subs;
	/** Emergency subcribers. */
	il_net_emcy_subscriber_lst_t emcy_subs;
	/** Operations. */
	const il_net_ops_t *ops;
};

/** Network device monitor. */
struct il_net_dev_mon {
	/** Operations. */
	const il_net_dev_mon_ops_t *ops;
};

/** Network implementations. */
#ifdef IL_HAS_PROT_EUSB
extern const il_net_ops_t il_eusb_net_ops;
extern const il_net_dev_mon_ops_t il_eusb_net_dev_mon_ops;
il_net_dev_list_t *il_eusb_net_dev_list_get(void);
#endif

#ifdef IL_HAS_PROT_MCB
extern const il_net_ops_t il_mcb_net_ops;
extern const il_net_dev_mon_ops_t il_mcb_net_dev_mon_ops;
il_net_dev_list_t *il_mcb_net_dev_list_get(void);
#endif

#ifdef IL_HAS_PROT_ETH
extern const il_eth_net_ops_t il_eth_net_ops;
extern const il_net_dev_mon_ops_t il_eth_net_dev_mon_ops;
il_eth_net_dev_list_t *il_eth_net_dev_list_get(void);
#endif

#ifdef IL_HAS_PROT_VIRTUAL
extern const il_net_ops_t il_virtual_net_ops;
extern const il_net_dev_mon_ops_t il_virtual_net_dev_mon_ops;
il_net_dev_list_t *il_virtual_net_dev_list_get(void);
#endif

#endif
