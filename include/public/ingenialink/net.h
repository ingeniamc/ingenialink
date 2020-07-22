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

#include <stdbool.h>

#include "common.h"
#include "registers.h"

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
	/** ETH. */
	IL_NET_PROT_ETH,
	/** ECAT. */
	IL_NET_PROT_ECAT,
	/** Virtual. */
	IL_NET_PROT_VIRTUAL,
} il_net_prot_t;

/** Network initialization options. */
typedef struct {
	/** Port. */
	const char *address_ip;
	/** Port. */
	const char *port;
	/** Port IP. */
	int port_ip;
	/** Read timeout (ms). */
	int timeout_rd;
	/** Write timeout (ms). */
	int timeout_wr;
	/** Connect to slave */
	int connect_slave;
	/** Connect to slave */
	int protocol;
} il_net_opts_t;


/** Ethernet network initialization options. */
typedef struct {
	/** Address ip. */
	const char *address_ip;
	/** Port. */
	const char *port;
	/** Port. */
	int port_ip;
	/** Read timeout (ms). */
	int timeout_rd;
	/** Write timeout (ms). */
	int timeout_wr;
	/** Connect to slave */
	int connect_slave;
	/** Protocol. */
	int protocol;
} il_eth_net_opts_t;

/** Ethercat network initialization options. */
typedef struct {
	/** Address ip. */
	const char *address_ip;
	/** Port. */
	const char *port;
	/** Port. */
	int port_ip;
	/** Read timeout (ms). */
	int timeout_rd;
	/** Write timeout (ms). */
	int timeout_wr;
	/** Connect to slave */
	int connect_slave;
	/** Protocol. */
	int protocol;
} il_ecat_net_opts_t;

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
typedef struct il_eth_net_dev_list {
	/** Port. */
	const char *address_ip;
	/** Next device */
	struct il_eth_net_dev_list *next;
} il_eth_net_dev_list_t;

typedef struct il_ecat_net_dev_list {
	/** Port. */
	const char *address_ip;
	/** Next device */
	struct il_ecat_net_dev_list *next;
} il_ecat_net_dev_list_t;


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

typedef struct il_eth_net_servos_list {
	/** Node id. */
	uint8_t id;
	/** Next node. */
	struct il_net_servos_list *next;
} il_eth_net_servos_list_t;

typedef struct il_ecat_net_servos_list {
	/** Node id. */
	uint8_t id;
	/** Next node. */
	struct il_net_servos_list *next;
} il_ecat_net_servos_list_t;

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
 * Obtain network status.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network is status.
 */
IL_EXPORT il_net_prot_t il_net_status_get(il_net_t *net);

/**
 * Stop network monitor.
 *
 * @param [in] net
 *	  Network.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_net_mon_stop(il_net_t *net);

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
 * Check if there is any slave connected.
 *
 * @param [in] net
 *	  Network.
 * @param [in] ip
 *	  Address ip.
 *	
 */
IL_EXPORT int *il_net_is_slave_connected(il_net_t *net, const char *ip);



/**
 * Obtain network extended data.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network extended data.
 */
IL_EXPORT char *il_net_extended_buffer_get(il_net_t *net);


/**
 * Obtain network monitoring data.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network monitoring data.
 */
IL_EXPORT uint16_t *il_net_monitornig_data_get(il_net_t *net);

/**
 * Obtain network monitoring data size.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network monitoring datasize.
 */
IL_EXPORT uint16_t il_net_monitornig_data_size_get(il_net_t *net);

/**
 * Obtain monitoring bytes per block.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Monitoring bytes per block.
 */
IL_EXPORT uint16_t il_net_monitornig_bytes_per_block_get(il_net_t *net);

/**
 * Obtain network disturbance data.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network disturbance data.
 */
IL_EXPORT uint16_t *il_net_disturbance_data_get(il_net_t *net);

/**
 * Obtain network disturbance data size.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Network disturbance datasize.
 */
IL_EXPORT uint16_t il_net_disturbance_data_size_get(il_net_t *net);

/**
 * Set network disturbance data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_set(il_net_t *net, uint16_t disturbance_data[2048]);

/**
 * Set network disturbance data size.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_size_set(il_net_t *net, uint16_t disturbance_data_size);

/**
 * Remove all Mapped registers.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT int *il_net_remove_all_mapped_registers(il_net_t *net);

/**
 * Set Mapped register.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *	
 * @param [in] address
 *	  Address.
 *
 *  @param [in] dtype
 *	  Data Type.
 * 
 */
IL_EXPORT int *il_net_set_mapped_register(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype);


/**
 * Obtain number of mapped registers.
 *
 * @param [in] net
 *	  Network.
 *
 * @returns
 *	Number of mapped registers.
 */
IL_EXPORT uint16_t il_net_num_mapped_registers_get(il_net_t *net);
/**
 * Enable monitoring.
 *
 * @param [in] net
 *	  Network.
 * 
 */ 
IL_EXPORT int *il_net_enable_monitoring(il_net_t *net);

/**
 * Disable monitoring.
 *
 * @param [in] net
 *	  Network.
 * 
 */ 
IL_EXPORT int *il_net_disable_monitoring(il_net_t *net);

/**
 * Read monitoring data.
 *
 * @param [in] net
 *	  Network.
 * 
 */ 
IL_EXPORT int *il_net_read_monitoring_data(il_net_t *net);

/**
 * Obtain network monitoring channel data.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *
 * @returns
 *	Network monitoring channel data.
 */
IL_EXPORT uint16_t *il_net_monitoring_channel_u16(il_net_t *net, int channel);

/**
 * Obtain network monitoring channel data.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *
 * @returns
 *	Network monitoring channel data.
 */
IL_EXPORT int16_t *il_net_monitoring_channel_s16(il_net_t *net, int channel);


/**
 * Obtain network monitoring channel data.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *
 * @returns
 *	Network monitoring channel data.
 */
IL_EXPORT uint32_t *il_net_monitoring_channel_u32(il_net_t *net, int channel);

/**
 * Obtain network monitoring channel data.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *
 * @returns
 *	Network monitoring channel data.
 */
IL_EXPORT int32_t *il_net_monitoring_channel_s32(il_net_t *net, int channel);

/**
 * Obtain network monitoring channel data.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *
 * @returns
 *	Network monitoring channel data.
 */
IL_EXPORT float *il_net_monitoring_channel_flt(il_net_t *net, int channel);

/**
 * Remove all Mapped registers from disturbance.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT int *il_net_disturbance_remove_all_mapped_registers(il_net_t *net);

/**
 * Set Mapped register for disturbance.
 *
 * @param [in] net
 *	  Network.
 *
 * @param [in] channel
 *	  Channel.
 *	
 * @param [in] address
 *	  Address.
 *
 *  @param [in] dtype
 *	  Data Type.
 * 
 */
IL_EXPORT int *il_net_disturbance_set_mapped_register(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype);

/**
 * Set network disturbance uint16 data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_u16_set(il_net_t *net, int channel, uint16_t disturbance_data[2048]);

/**
 * Set network disturbance int16 data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_s16_set(il_net_t *net, int channel, int16_t disturbance_data[2048]);

/**
 * Set network disturbance uint32 data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_u32_set(il_net_t *net, int channel, uint32_t disturbance_data[2048]);

/**
 * Set network disturbance int32 data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_s32_set(il_net_t *net, int channel, int32_t disturbance_data[2048]);

/**
 * Set network disturbance float data.
 *
 * @param [in] net
 *	  Network.
 *	
 */
IL_EXPORT void il_net_disturbance_data_flt_set(il_net_t *net, int channel, float disturbance_data[2048]);

/**
 * Close socket connected.
 *
 * @param [in] net
 *	  Network.
 * 
 */
IL_EXPORT int il_net_close_socket(il_net_t *net);

/**
	SOEM
*/
IL_EXPORT int il_net_master_startup(il_net_t **net, char *ifname, const char *if_address_ip);

IL_EXPORT int il_net_master_stop(il_net_t **net);

IL_EXPORT int il_net_update_firmware(il_net_t **net, char *ifname, uint16_t slave, char *filename, bool is_summit);

IL_EXPORT int il_net_eeprom_tool(il_net_t **net, char *ifname, int slave, int mode, char *fname);

IL_EXPORT int il_net_force_error(il_net_t **net, char *ifname, char *if_address_ip);

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

IL_EXPORT void il_net_fake_destroy(il_net_t *net);

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
