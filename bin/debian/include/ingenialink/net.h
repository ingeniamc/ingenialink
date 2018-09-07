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

/** Network. */
typedef struct il_net il_net_t;

/** Network protocols. */
typedef enum {
	/** E-USB. */
	IL_NET_PROT_EUSB,
	/** MCB. */
	IL_NET_PROT_MCB,
} il_net_prot_t;

/** Network initialization options. */
typedef struct {
	/** Port. */
	const char *port;
	/** Read timeout (ms). */
	int timeout_rd;
	/** Write timeout (ms). */
	int timeout_wr;
} il_net_opts_t;

/** Default read timeout (ms). */
#define IL_NET_TIMEOUT_RD_DEF	500

/** Default write timeout (ms). */
#define IL_NET_TIMEOUT_WR_DEF	500

/** Network state. */
typedef enum {
	/** Connected. */
	IL_NET_STATE_CONNECTED,
	/** Disconnected. */
	IL_NET_STATE_DISCONNECTED,
	/** Faulty (e.g. device was physically disconnected). */
	IL_NET_STATE_FAULTY,
} il_net_state_t;

/** Port maximum size. */
#define IL_NET_PORT_SZ 128U

/** network devices list. */
typedef struct il_net_dev_list {
	/** Port. */
	char port[IL_NET_PORT_SZ];
	/** Next device */
	struct il_net_dev_list *next;
} il_net_dev_list_t;

/** Network servos list. */
typedef struct il_net_servos_list {
	/** Node id. */
	uint8_t id;
	/** Next node. */
	struct il_net_servos_list *next;
} il_net_servos_list_t;

/** Node found callback. */
typedef void (*il_net_servos_on_found_t)(void *ctx, uint8_t id);

/** Device monitor event types. */
typedef enum {
	/** Device added */
	IL_NET_DEV_EVT_ADDED,
	/** Device removed */
	IL_NET_DEV_EVT_REMOVED
} il_net_dev_evt_t;

/** Network device monitor */
typedef struct il_net_dev_mon il_net_dev_mon_t;

/** Network device event callback. */
typedef void (*il_net_dev_on_evt_t)(void *ctx, il_net_dev_evt_t evt,
				      const char *port);

/**
 * Create a network.
 *
 * @notes
 *	Network is connected when created.
 *
 * @param [in] prot
 *	Protocol.
 * @param [in] opts
 *	Initialization options.
 *
 * @return
 *	Network  (NULL if it could not be created).
 */
IL_EXPORT il_net_t *il_net_create(il_net_prot_t prot,
				  const il_net_opts_t *opts);

/**
 * Destroy a network.
 *
 * @param [in] net
 *	  Network.
 */
IL_EXPORT void il_net_destroy(il_net_t *net);

/**
 * Connect network.
 *
 * @param [in] net
 *	  Network.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_net_connect(il_net_t *net);

/**
 * Disconnect network.
 *
 * @param [in] net
 *	  Network.
 */
IL_EXPORT void il_net_disconnect(il_net_t *net);

/**
 * Obtain network protocol.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network protocol.
 */
IL_EXPORT il_net_prot_t il_net_prot_get(il_net_t *net);

/**
 * Obtain network state.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network state.
 */
IL_EXPORT il_net_state_t il_net_state_get(il_net_t *net);

/**
 * Obtain network port.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network port.
 */
IL_EXPORT const char *il_net_port_get(il_net_t *net);

/**
 * Obtain network servos list.
 *
 * @note
 *	A callback can be given to obtain *real-time* servos information. This
 *	may be useful for GUIs.
 *
 * @param [in] net
 *	network.
 * @param [in] on_found
 *	Callback that will be called every time a node is found (optional).
 * @param [in] ctx
 *	Callback context (optional).
 *
 * @returns
 *	Network servos list (NULL if none are found or any error occurs).
 *
 * @see
 *	il_net_servos_list_destroy
 */
IL_EXPORT il_net_servos_list_t *il_net_servos_list_get(
		il_net_t *net, il_net_servos_on_found_t on_found, void *ctx);

/**
 * Destroy network servos list.
 *
 * @param [in, out] lst
 *	Network servos list.
 *
 * @see
 *	il_net_servos_list_get
 */
IL_EXPORT void il_net_servos_list_destroy(il_net_servos_list_t *lst);

/** Utility macro to iterate over a list of network servos list. */
#define il_net_servos_list_foreach(item, lst) \
	for ((item) = (lst); (item); (item) = (item)->next)

/**
 * Create a network device monitor.
 *
 * @param [in] prot
 *	Protocol.
 *
 * @return
 *      An  of a device monitor (NULL if it could not be created).
 */
IL_EXPORT il_net_dev_mon_t *il_net_dev_mon_create(il_net_prot_t prot);

/**
 * Start the network device monitor.
 *
 * @param [in] mon
 *      Monitor.
 * @param [in] on_evt
 *      Callback function that will be called when a new device is added or
 *      removed.
 * @param [in] ctx
 *      Callback context (optional).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_net_dev_mon_start(il_net_dev_mon_t *mon,
				   il_net_dev_on_evt_t on_evt, void *ctx);

/**
 * Stop the network device monitor.
 *
 * @param [in] mon
 *      Monitor.
 */
IL_EXPORT void il_net_dev_mon_stop(il_net_dev_mon_t *mon);

/**
 * Destroy the network device monitor.
 *
 * @note
 *	If the monitor is running, it will be stopped.
 *
 * @param [in] mon
 *      Monitor.
 */
IL_EXPORT void il_net_dev_mon_destroy(il_net_dev_mon_t *mon);

/**
 * Obtain network devices list.
 *
 * @param [in] prot
 *	Protocol.
 *
 * @returns
 *	Network devices list (NULL if none are found or any error occurs).
 *
 * @see
 *	il_net_dev_list_destroy
 */
IL_EXPORT il_net_dev_list_t *il_net_dev_list_get(il_net_prot_t prot);

/**
 * Destroy Network device list.
 *
 * @param [in, out] lst
 *	Network device list.
 *
 * @see
 *	il_net_dev_list_get
 */
IL_EXPORT void il_net_dev_list_destroy(il_net_dev_list_t *lst);

/** Utility macro to iterate over a list of network devices. */
#define il_net_dev_list_foreach(item, lst) \
	for ((item) = (lst); (item); (item) = (item)->next)

/** @} */

IL_END_DECL

#endif
