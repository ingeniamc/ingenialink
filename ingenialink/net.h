#ifndef NET_H_
#define NET_H_

#include "ingenialink/net.h"
#include "ingenialink/utils.h"
#include "ingenialink/servo.h"

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

struct monitoring_data_t {
	il_reg_dtype_t type;
	union {
		uint8_t monitoring_data_u8[1024];
		int8_t monitoring_data_s8[1024];
		uint16_t monitoring_data_u16[1024];
		int16_t monitoring_data_s16[1024];
		uint32_t monitoring_data_u32[1024];
		int32_t monitoring_data_s32[1024];
		uint64_t monitoring_data_u64[1024];
		int64_t monitoring_data_s64[1024];
		float monitoring_data_flt[102400];
	} value;
};

struct disturbance_data_t {
	il_reg_dtype_t type;
	union {
		uint8_t disturbance_data_u8[1024];
		int8_t disturbance_data_s8[1024];
		uint16_t disturbance_data_u16[1024];
		int16_t disturbance_data_s16[1024];
		uint32_t disturbance_data_u32[1024];
		int32_t disturbance_data_s32[1024];
		float disturbance_data_flt[1024];
	} value;
};

/** Network. */
struct il_net {
	/** Protocol */
	il_net_prot_t prot;
	/** Port */
	char *port;
	/** Port IP */
	int port_ip;
	/** Address Ip */
	char *address_ip;
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
	/** Monitoring Raw Data. */
	uint8_t monitoring_raw_data[409600];
	/** Extended buffer **/
	char extended_buff[128];
	/** Monitoring Data. */
	struct monitoring_data_t monitoring_data_channels[15];
	/** Monitoring number of mapped registers */
	uint16_t monitoring_number_mapped_registers;
	/** Monitoring bytes per block */
	uint16_t monitoring_bytes_per_block;
	/** Monitoring Data size. */
	uint32_t monitoring_data_size;
	/** Disturbance Raw Data. */
	uint16_t disturbance_data[2048];
	/** Distburbance Data. */
	struct disturbance_data_t disturbance_data_channels[16];
	/** Disturbance Data size. */
	uint32_t disturbance_data_size;
	/** Last disturbance channel */
	uint8_t last_channel;
	uint16_t disturbance_number_mapped_registers;

	int slave;
	/** Operations. */
	const il_net_ops_t *ops;
};

/** Network device monitor. */
struct il_net_dev_mon {
	/** Operations. */
	const il_net_dev_mon_ops_t *ops;
};

/** Network implementations. */
#ifdef IL_HAS_PROT_ETH
extern const il_eth_net_ops_t il_eth_net_ops;
extern const il_net_dev_mon_ops_t il_eth_net_dev_mon_ops;
il_eth_net_dev_list_t *il_eth_net_dev_list_get(void);
#endif


extern const il_ecat_net_ops_t il_ecat_net_ops;
extern const il_net_dev_mon_ops_t il_ecat_net_dev_mon_ops;
il_ecat_net_dev_list_t *il_ecat_net_dev_list_get(void);

#endif
