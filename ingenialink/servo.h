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

#include "public/ingenialink/servo.h"

#include "ingenialink/net.h"
#include "osal/osal.h"

/** Minimum servo id. */
#define SERVOID_MIN		1

/** Maximum servo id. */
#define SERVOID_MAX		127

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** Set-point acknowledge timeout (ms). */
#define SPACK_TIMEOUT		1000

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
	/** Servo id. */
	uint8_t id;
	/** Communications timeout (ms). */
	int timeout;
	/** Units. */
	il_servo_units_t units;
	/** Configuration. */
	il_servo_cfg_t cfg;
	/** Statusword subscription. */
	il_servo_sw_t sw;
};

#endif
