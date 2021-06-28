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

#ifndef INGENIALINK_NET_H_
#define INGENIALINK_NET_H_

#include "public/ingenialink/net.h"
#include "public/ingenialink/servo.h"

/** Virtual network port. */
#define EUSB_VIRTUAL_PORT "virtual"

/** Virtual network servo ID. */
#define EUSB_VIRTUAL_ID 0x01

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
 * Set state of the network.
 *
 * @param [in] net
 *	Network.
 * @param [in] state
 *	State.
 */
void il_net__state_set(il_net_t *net, il_net_state_t state);

/**
 * Write.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Node id.
 * @param [in] subnode
 *	Subnode.
 * @param [in] address
 *	Address.
 * @param [in] buf
 *	Data buffer (optional).
 * @param [in] sz
 *	Data buffer size.
 * @param [in] confirmed
 *	Flag to confirm the write.
 *
 * @returns
 *	0 on success, error code otherwise.
 */
int il_net__write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, const void *buf,
		  size_t sz, int confirmed);

/**
 * Read.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Expected node id (0 to match any).
 * @param [in] subnode
 *	Subnode.
 * @param [in] address
 *	Expected address.
 * @param [out] buf
 *	Data output buffer.
 * @param [in] sz
 *	Data buffer size.
 *
 * @returns
 *	0 on success, error code otherwise.
 */
int il_net__read(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, void *buf,
		 size_t sz);

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
int il_net__sw_subscribe(il_net_t *net, uint16_t id,
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
int il_net__emcy_subscribe(il_net_t *net, uint16_t id,
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

/** Network operations. */
typedef struct {
	/** Retain. */
	void (*_retain)(
		il_net_t *net);
	/** Release. */
	void (*_release)(
		il_net_t *net);

	/** Set state. */
	void (*_state_set)(
		il_net_t *net, il_net_state_t state);
	/** Read. */
	int (*_read)(
		il_net_t *net, uint16_t id, uint32_t address, void *buf,
		size_t sz);
	/** Write. */
	int (*_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);
	/** Wait Write. */
	int (*_wait_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);
	/** Subscribe to state updates. */
	int (*_sw_subscribe)(
		il_net_t *net, uint16_t id, il_net_sw_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe from state updares. */
	void (*_sw_unsubscribe)(
		il_net_t *net, int slot);
	/** Subscribe to emergencies. */
	int (*_emcy_subscribe)(
		il_net_t *net, uint16_t id, il_net_emcy_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe to emergencies. */
	void (*_emcy_unsubscribe)(
		il_net_t *net, int slot);
	/** Create network. */
	il_net_t *(*create)(
		const il_net_opts_t *opts);
	/** Destroy network. */
	void (*destroy)(
		il_net_t *net);
	/** Connect network. */
	int (*connect)(
		il_net_t *net);
	/** Disconnect network. */
	void (*disconnect)(
		il_net_t *net);
	/** Obtain network state. */
	il_net_state_t (*state_get)(
		il_net_t *net);
	/** Obtain network status. */
	il_net_state_t (*status_get)(
		il_net_t *net);
	/** Stop monitor. */
	int (*mon_stop)(
		il_net_t *net);
	/** Obtain list of connected servos. */
	il_net_servos_list_t *(*servos_list_get)(
		il_net_t *net, il_net_servos_on_found_t on_found, void *ctx);
	/** Monitoring. */
	int (*remove_all_mapped_registers)();
	int (*set_mapped_register)();
	int (*enable_monitoring)();
	int (*disable_monitoring)();
	int (*read_monitoring_data)();
	int (*recv_monitoring)();
	/** Disturbance. */
	int (*disturbance_remove_all_mapped_registers)();
	int (*disturbance_set_mapped_register)();
} il_net_ops_t;


/** Ethernet network operations. */
typedef struct {
	/** Retain. */
	void (*_retain)(
		il_net_t *net);
	/** Release. */
	void (*_release)(
		il_net_t *net);

	/** Set state. */
	void (*_state_set)(
		il_net_t *net, il_net_state_t state);
	/** Read. */
	int (*_read)(
		il_net_t *net, uint16_t id, uint32_t address, void *buf,
		size_t sz);
	/** Write. */
	int (*_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);

	/** Write. */
	int (*_wait_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);
	/** Subscribe to state updates. */
	int (*_sw_subscribe)(
		il_net_t *net, uint16_t id, il_net_sw_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe from state updares. */
	void (*_sw_unsubscribe)(
		il_net_t *net, int slot);
	/** Subscribe to emergencies. */
	int (*_emcy_subscribe)(
		il_net_t *net, uint16_t id, il_net_emcy_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe to emergencies. */
	void (*_emcy_unsubscribe)(
		il_net_t *net, int slot);
	/** Create network. */
	il_net_t *(*create)(
		const il_net_opts_t *opts);
	/** Destroy network. */
	void (*destroy)(
		il_net_t *net);
	/** Connect network. */
	int (*connect)(
		il_net_t *net);
	/** Disconnect network. */
	void (*disconnect)(
		il_net_t *net);
	/** Obtain network state. */
	il_net_state_t (*state_get)(
		il_net_t *net);
	/** Obtain network status. */
	il_net_state_t (*status_get)(
		il_net_t *net);
	/** Stop monitor. */
	int (*mon_stop)(
		il_net_t *net);
	/** Obtain list of connected servos. */
	il_eth_net_servos_list_t *(*servos_list_get)(
		il_net_t *net, il_net_servos_on_found_t on_found, void *ctx);
	/** Monitoring. */
	int (*remove_all_mapped_registers)();
	int (*set_mapped_register)();
	int (*enable_monitoring)();
	int (*disable_monitoring)();
	int (*read_monitoring_data)();
	int (*recv_monitoring)();
	/** Disturbance. */
	int (*disturbance_remove_all_mapped_registers)();
	int (*disturbance_set_mapped_register)();
	int (*set_last_channel)();
	/** Is Slave Connected. */
	int (*is_slave_connected)();
	/** Close socket */
	int (*close_socket)();
	int (*set_reconnection_retries)();
	int (*set_recv_timeout)();
	int (*set_status_check_stop)();
	int (*SDO_read)();
	int (*SDO_read_string)();
	int (*SDO_write)();
} il_eth_net_ops_t;

/** Network device monitor operations. */
typedef struct {
	/** Create. */
	il_net_dev_mon_t *(*create)(void);
	/** Destroy. */
	void (*destroy)(
		il_net_dev_mon_t *mon);
	/** Start. */
	int (*start)(
		il_net_dev_mon_t *mon, il_net_dev_on_evt_t on_evt, void *ctx);
	/** Stop. */
	void (*stop)(
		il_net_dev_mon_t *mon);
} il_net_dev_mon_ops_t;

/** Ethercat network operations. */
typedef struct {
	/** Retain. */
	void (*_retain)(
		il_net_t *net);
	/** Release. */
	void (*_release)(
		il_net_t *net);

	/** Set state. */
	void (*_state_set)(
		il_net_t *net, il_net_state_t state);
	/** Read. */
	int (*_read)(
		il_net_t *net, uint16_t id, uint32_t address, void *buf,
		size_t sz);
	/** Write. */
	int (*_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);

	/** Write. */
	int (*_wait_write)(
		il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		size_t sz, int confirmed);
	/** Subscribe to state updates. */
	int (*_sw_subscribe)(
		il_net_t *net, uint16_t id, il_net_sw_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe from state updares. */
	void (*_sw_unsubscribe)(
		il_net_t *net, int slot);
	/** Subscribe to emergencies. */
	int (*_emcy_subscribe)(
		il_net_t *net, uint16_t id, il_net_emcy_subscriber_cb_t cb,
		void *ctx);
	/** Unsubscribe to emergencies. */
	void (*_emcy_unsubscribe)(
		il_net_t *net, int slot);
	/** Create network. */
	il_net_t *(*create)(
		const il_net_opts_t *opts);
	/** Destroy network. */
	void (*destroy)(
		il_net_t *net);
	/** Connect network. */
	int (*connect)(
		il_net_t *net);
	/** Disconnect network. */
	void (*disconnect)(
		il_net_t *net);
	/** Obtain network state. */
	il_net_state_t (*state_get)(
		il_net_t *net);
	/** Obtain network status. */
	il_net_state_t (*status_get)(
		il_net_t *net);
	/** Stop monitor. */
	int (*mon_stop)(
		il_net_t *net);
	/** Obtain list of connected servos. */
	il_ecat_net_servos_list_t *(*servos_list_get)(
		il_net_t *net, il_net_servos_on_found_t on_found, void *ctx);
	/** Monitoring. */
	int (*remove_all_mapped_registers)();
	int (*set_mapped_register)();
	int (*enable_monitoring)();
	int (*disable_monitoring)();
	int (*read_monitoring_data)();
	int (*recv_monitoring)();
	/** Disturbance. */
	int (*disturbance_remove_all_mapped_registers)();
	int (*disturbance_set_mapped_register)();
	int (*set_last_channel)();
	/** Is Slave Connected. */
	int (*is_slave_connected)();
	/** Close socket */
	int (*close_socket)();
	int (*master_startup)();
	int (*num_slaves_get)();
	int (*master_stop)();
	int (*update_firmware)();
	int (*eeprom_tool)();
	int (*force_error)();
	int (*set_if_params)();
	int (*set_reconnection_retries)();
	int (*set_recv_timeout)();
	int (*set_status_check_stop)();
	int (*net_test)();
	int (*SDO_read)();
	int (*SDO_read_string)();
	int (*SDO_write)();
} il_ecat_net_ops_t;

#endif
