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

#ifndef AXIS_H
#define AXIS_H

#include "public/ingenialink/axis.h"

#include "ingenialink/net.h"
#include "osal/osal.h"

/** Minimum axis id. */
#define AXISID_MIN		1

/** Maximum axis id. */
#define AXISID_MAX		127

/** PDS timeout (ms). */
#define PDS_TIMEOUT		1000

/** Set-point acknowledge timeout (ms). */
#define SPACK_TIMEOUT		1000

/*
 * Types of velocity and position feedbacks and associated constants.
 *
 * References:
 *	http://doc.ingeniamc.com/display/i14402/0x2310+-+Feedbacks
 */
#define FB_VEL_POS		0x00
#define FB_VEL_TACHOMETER	0x01

#define FB_POS_DIGITAL_ENCODER	0x00
#define FB_POS_DIGITAL_HALLS	0x01
#define FB_POS_ANALOG_HALLS	0x02
#define FB_POS_ANALOG_INPUT	0x04
#define FB_POS_SSI		0x05
#define FB_POS_SINCOS		0x06
#define FB_POS_PWM		0x07
#define FB_POS_RESOLVER		0x08
#define FB_POS_NONE		0x09
#define FB_POS_SIMULATED	0x0B

#define DIGITAL_HALLS_CONSTANT	6
#define ANALOG_HALLS_CONSTANT	4096
#define ANALOG_INPUT_CONSTANT	4096
#define SINCOS_CONSTANT		1024
#define PWM_CONSTANT		65535
#define RESOLVER_CONSTANT	65535

/** Axis units. */
typedef struct {
	/** Torque. */
	il_units_torque_t torque;
	/** Position. */
	il_units_pos_t pos;
	/** Velocity. */
	il_units_vel_t vel;
	/** Acceleration. */
	il_units_acc_t acc;
} il_axis_units_t;

/** Axis configuration. */
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
} il_axis_cfg_t;

/** Statusword updates subcription. */
typedef struct {
	/** Value. */
	uint16_t value;
	/** Lock. */
	osal_mutex_t *lock;
	/** Changed condition. */
	osal_cond_t *changed;
} il_axis_sw_t;

/** IngeniaLink axis. */
struct il_axis {
	/** Associated IngeniaLink network. */
	il_net_t *net;
	/** Axis id. */
	uint8_t id;
	/** Communications timeout (ms). */
	int timeout;
	/** Units. */
	il_axis_units_t units;
	/** Configuration. */
	il_axis_cfg_t cfg;
	/** Statusword subscription. */
	il_axis_sw_t sw;
};

#endif
