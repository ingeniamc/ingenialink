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

#ifndef PUBLIC_INGENIALINK_NET_H_
#define PUBLIC_INGENIALINK_NET_H_

#include "common.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/net.h
 * @brief Network.
 * @defgroup IL_NET Network
 * @ingroup IL
 * @{
 */

/** IngeniaLink network instance. */
typedef struct il_net il_net_t;

/** Default IngeniaLink communications timeout (ms). */
#define IL_NET_TIMEOUT_DEF	100

/** IngeniaLink port maximum size. */
#define IL_NET_PORT_SZ		128U

/** IngeniaLink network devices list. */
typedef struct il_net_dev_list {
	/** Port. */
	char port[IL_NET_PORT_SZ];
	/** Next device */
	struct il_net_dev_list *next;
} il_net_dev_list_t;

/** IngeniaLink network nodes list. */
typedef struct il_net_nodes_list {
	/** Node id. */
	uint8_t id;
	/** Next node. */
	struct il_net_nodes_list *next;
} il_net_nodes_list_t;

/** IngeniaLink node found callback. */
typedef void (*il_net_nodes_on_found_t)(void *ctx, uint8_t id);

/** Device monitor event types. */
typedef enum {
	/** Device added */
	IL_NET_DEV_EVT_ADDED,
	/** Device removed */
	IL_NET_DEV_EVT_REMOVED
} il_net_dev_evt_t;

/** IngeniaLink network device monitor */
typedef struct il_net_dev_mon il_net_dev_mon_t;

/** IngeniaLink network device event callback. */
typedef void (*il_net_dev_on_evt_t)(void *ctx, il_net_dev_evt_t evt,
				      const char *port);

/**
 * Create IngeniaLink network instance.
 *
 * @param [in] port
 *	Port.
 * @param [in] timeout
 *	Communications timeout (ms, must be > 0).
 *
 * @return
 *	  Network instance (NULL if it could not be created).
 */
IL_EXPORT il_net_t *il_net_create(const char *port, unsigned int timeout);

/**
 * Destroy an IngeniaLink network instance.
 *
 * @note
 *	If the network is up, it will be brought down before destroying it.
 *
 * @param [in] net
 *	  IngeniaLink network instance.
 */
IL_EXPORT void il_net_destroy(il_net_t *net);

/**
 * Obtain IngeniaLink network devices list.
 *
 * @returns
 *	IngeniaLink network devices list (NULL if none are found or any error
 *	occurs).
 *
 * @see
 *	il_net_dev_list_destroy
 */
IL_EXPORT il_net_dev_list_t *il_net_dev_list_get(void);

/**
 * Destroy IngeniaLink network device list.
 *
 * @param [in, out] lst
 *	IngeniaLink network device list.
 *
 * @see
 *	il_net_dev_list_get
 */
IL_EXPORT void il_net_dev_list_destroy(il_net_dev_list_t *lst);

/** Utility macro to iterate over a list of IngeniaLink network devices. */
#define il_net_dev_list_foreach(item, lst) \
	for ((item) = (lst); (item); (item) = (item)->next)

/**
 * Create an IngeniaLink network device monitor.
 *
 * @param [in] on_evt
 *      Callback function that will be called when a new device is added or
 *      removed.
 * @param [in] ctx
 *      Callback context (optional).
 *
 * @return
 *      An instance of the running device monitor (NULL if it could not be
 *      created).
 */
IL_EXPORT il_net_dev_mon_t *il_net_dev_mon_create(
		il_net_dev_on_evt_t on_evt, void *ctx);

/**
 * Destroy the IngeniaLink network device monitor.
 *
 * @param [in] mon
 *      Monitor instance.
 */
IL_EXPORT void il_net_dev_mon_destroy(il_net_dev_mon_t *mon);

/**
 * Obtain IngeniaLink network nodes list.
 *
 * @note
 *	A callback can be given to obtain *real-time* nodes information. This
 *	may be useful for GUIs.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] on_found
 *	Callback that will be called every time a node is found (optional).
 * @param [in] ctx
 *	Callback context (optional).
 *
 * @returns
 *	IngeniaLink network nodes list (NULL if none are found or any error
 *	occurs).
 *
 * @see
 *	il_net_nodes_list_destroy
 */
IL_EXPORT il_net_nodes_list_t *il_net_nodes_list_get(
		il_net_t *net, il_net_nodes_on_found_t on_found, void *ctx);

/**
 * Destroy IngeniaLink network nodes list.
 *
 * @param [in, out] lst
 *	IngeniaLink network nodes list.
 *
 * @see
 *	il_net_nodes_list_get
 */
IL_EXPORT void il_net_nodes_list_destroy(il_net_nodes_list_t *lst);

/** Utility macro to iterate over a list of IngeniaLink network nodes list. */
#define il_net_nodes_list_foreach(item, lst) \
	for ((item) = (lst); (item); (item) = (item)->next)

/** @} */

IL_END_DECL

#endif
