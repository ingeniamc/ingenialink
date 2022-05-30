#ifndef MCB_NET_H_
#define MCB_NET_H_

#include "../net.h"

#include "ingenialink/utils.h"

#include "osal/osal.h"

#define _SER_NO_LEGACY_STDINT
#include <sercomm/sercomm.h>

/** Default baudrate. */
#define BAUDRATE_DEF		115200

/** Default read timeout. */
#define READ_TIMEOUT_DEF	400000

/** Vendor ID register address. */
#define VENDOR_ID_ADDR		0x06E0

/** Statusword address. */
#define STATUSWORD_ADDRESS	0x0011


/** MCB network. */
typedef struct il_mcb_net {
	/** Network (parent). */
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
	// il_eusb_net_sync_t sync;
} il_mcb_net_t;

/** MCB network device monitor */
typedef struct il_mcb_net_dev_mon {
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
} il_mcb_net_dev_mon_t;

/** Obtain MCB Network from parent. */
#define to_mcb_net(ptr) container_of(ptr, struct il_mcb_net, net)

/** Obtain MCB Network device monitor from parent. */
#define to_mcb_mon(ptr) container_of(ptr, struct il_mcb_net_dev_mon, mon)

#endif
