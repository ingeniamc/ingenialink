#ifndef ETH_NET_H_
#define ETH_NET_H_

#include "../net.h"

#include "ingenialink/utils.h"

#include "osal/osal.h"

#define _SER_NO_LEGACY_STDINT
#include <winsock2.h>

/** Default number of retries while waiting to receive a frame. */
#define NUMBER_OP_RETRIES_DEF 	0

/** Default read timeout. */
#define READ_TIMEOUT_DEF	400000

/** Default reconnection retries. */
#define RECONNECTION_RETRIES_DEF	7

/** Vendor ID register address. */
#define VENDOR_ID_ADDR		0x06E0

/** Statusword address. */
#define STATUSWORD_ADDRESS	0x0011

/** Product code COCO. */
#define PRODUCT_CODE_COCO	0x06E1

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
	/** Check status */
	int status_check_stop;
	/** Listener thread. */
	osal_thread_t *listener;
	/** Listener stop flag. */
	int stop;
	/** Protocol. */
	int protocol;
	/** Reconnection retries. */
	uint8_t reconnection_retries;
	/** Recv timeout in ms*/
	uint32_t recv_timeout;

	uint8_t use_eoe_comms;

} il_eth_net_t;

/** ETH network device monitor */
typedef struct il_eth_net_dev_mon {
	/** Network monitor (parent). */
	il_net_dev_mon_t mon;
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