#ifndef SERVO_H
#define SERVO_H

#include "ingenialink/servo.h"

#include "public/ingenialink/dict.h"
#include "ingenialink/net.h"
#include "ingenialink/utils.h"

#include "osal/osal.h"

/** State external subscribers default array size. */
#define STATE_SUBS_SZ_DEF	10

/** State external subscribers period timeout (ms). */
#define STATE_SUBS_TIMEOUT	100

/** Emergencies queue size. */
#define EMCY_QUEUE_SZ		4

/** Emergency external subscribers default array size. */
#define EMCY_SUBS_SZ_DEF	10

/** Emergency external subscribers monitor period timeout (ms). */
#define EMCY_SUBS_TIMEOUT	100

/** Servo units. */
typedef struct {
	/** Lock. */
	osal_mutex_t *lock;
	/** Torque. */
	il_units_torque_t torque;
	/** Position. */
	il_units_pos_t pos;
	/** Velocity. */
	il_units_vel_t vel;
	/** Acceleration. */
	il_units_acc_t acc;
} il_servo_units_t;

/** Servo configuration. */
typedef struct {
	/** Rated torque (N). */
	double rated_torque;
	/** Position resolution (counts/rev). */
	double pos_res;
	/** Velocity resolution (counts/rev/s). */
	double vel_res;
	/** Acceleration resolution (counts/rev/s^2). */
	double acc_res;
	/** Distance scale (m). */
	double dist_scale;
} il_servo_cfg_t;

/** Emergency subscriber. */
typedef struct {
	/** Callback. */
	il_servo_emcy_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
} il_servo_emcy_subscriber_t;

/** Emergencies subscribers list. */
typedef struct {
	/** Array of subscribers. */
	il_servo_emcy_subscriber_t *subs;
	/** Array size. */
	size_t sz;
	/** Lock. */
	osal_mutex_t *lock;
	/** Monitor. */
	osal_thread_t *monitor;
	/** Monitor stop flag. */
	int stop;
} il_servo_emcy_subscriber_lst_t;

/** Emergencies subcription. */
typedef struct {
	/** Queue head. */
	size_t head;
	/** Queue tail. */
	size_t tail;
	/** Queue size. */
	size_t sz;
	/** Queue. */
	uint32_t queue[EMCY_QUEUE_SZ];
	/** Lock. */
	osal_mutex_t *lock;
	/** Not empty condition. */
	osal_cond_t *not_empty;
	/** Assigned subscription slot. */
	int slot;
} il_servo_emcy_t;

/** State update subscriber. */
typedef struct {
	/** Callback. */
	il_servo_state_subscriber_cb_t cb;
	/** Callback context. */
	void *ctx;
} il_servo_state_subscriber_t;

/** State update subscribers list. */
typedef struct {
	/** Array of subscribers. */
	il_servo_state_subscriber_t *subs;
	/** Array size. */
	size_t sz;
	/** Lock. */
	osal_mutex_t *lock;
	/** Monitor. */
	osal_thread_t *monitor;
	/** Monitor stop flag. */
	int stop;
	int kill;
} il_servo_state_subscriber_lst_t;

/** Statusword updates subcription. */
typedef struct {
	/** Value. */
	uint16_t value;
	/** Lock. */
	osal_mutex_t *lock;
	/** Changed condition. */
	osal_cond_t *changed;
	/** Assigned subscription slot. */
	int slot;
} il_servo_sw_t;

/** IngeniaLink servo. */
struct il_servo {
	/** Associated IngeniaLink network. */
	il_net_t *net;
	/** ID. */
	uint16_t id;
	/** Subnodes. */
	uint16_t subnodes;
	/** Dictionary. */
	il_dict_t *dict;
	/** Units. */
	il_servo_units_t units;
	/** Configuration. */
	il_servo_cfg_t cfg;
	/** Operation mode. */
	il_servo_mode_t mode;
	/** Statusword subscription. */
	il_servo_sw_t sw;
	/** External state change subscriptors. */
	il_servo_state_subscriber_lst_t state_subs;
	/** Emergency subscription. */
	il_servo_emcy_t emcy;
	/** External emergency subscriptors. */
	il_servo_emcy_subscriber_lst_t emcy_subs;
	/** Operations. */
	const il_servo_ops_t *ops;
};

/** Servo implementations. */
#ifdef IL_HAS_PROT_ETH
extern const il_servo_ops_t il_eth_servo_ops;
#endif

#ifdef IL_HAS_PROT_ECAT
extern const il_servo_ops_t il_ecat_servo_ops;
#endif

#endif
