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

#ifndef SERVO_H
#define SERVO_H

#include "ingenialink/servo.h"

#include "ingenialink/net.h"
#include "ingenialink/utils.h"

#include "osal/osal.h"

/** Minimum servo id. */
#define SERVOID_MIN		1

/** Maximum servo id. */
#define SERVOID_MAX		127

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** State external subscribers default array size. */
#define STATE_SUBS_SZ_DEF	10

/** State external subscribers period timeout (ms). */
#define STATE_SUBS_TIMEOUT	100

/** Flags position offset in statusword. */
#define FLAGS_SW_POS		10

/** Emergencies queue size. */
#define EMCY_QUEUE_SZ		4

/** Emergency external subscribers default array size. */
#define EMCY_SUBS_SZ_DEF	10

/** Emergency external subscribers monitor period timeout (ms). */
#define EMCY_SUBS_TIMEOUT	100

/*
 * Constants associated to types of velocity and position feedbacks.
 *
 * References:
 *	http://doc.ingeniamc.com/display/i14402/0x2310+-+Feedbacks
 */

#define DIGITAL_HALLS_CONSTANT	6
#define ANALOG_HALLS_CONSTANT	4096
#define ANALOG_INPUT_CONSTANT	4096
#define SINCOS_CONSTANT		1024
#define PWM_CONSTANT		65535
#define RESOLVER_CONSTANT	65535

/** Relative voltage range. */
#define VOLT_REL_RANGE		32767

/** Radians range. */
#define RAD_RANGE		65535

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
	/** Pole pitch (m). */
	double ppitch;
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
	/** Reference counter. */
	refcnt_t *refcnt;
	/** Servo id. */
	uint8_t id;
	/** Communications timeout (ms). */
	int timeout;
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
};

#endif
