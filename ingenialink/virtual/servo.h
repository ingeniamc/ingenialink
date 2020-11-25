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

#ifndef VIRTUAL_SERVO_H
#define VIRTUAL_SERVO_H

#include "../servo.h"

/** Default subnode */
#define DFLT_SUBNODE		1

/** Randomness factor (% of amplitude). */
#define RANDOM_FACTOR		0.05

/** Temperature (mC). */
#define TEMPERATURE			30000

/** Voltage (mV). */
#define DC_VOLTAGE			24000

/** Registers that can be queried by address. */
#define ADDR_POS_ACT		0x6064
#define ADDR_VEL_ACT		0x606C
#define ADDR_TORQUE_TGT		0x6071
#define ADDR_TORQUE_ACT		0x6077
#define ADDR_POS_TGT		0x607A
#define ADDR_VEL_TGT		0x60FF
#define ADDR_TEMP_ACT		0x0120C2
#define ADDR_DC_VOLTAGE		0x6079

/** IngeniaLink servo. */
typedef struct il_virtual_servo {
	/** Servo (parent). */
	il_servo_t servo;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
	/** State store. */
	struct {
		il_servo_mode_t mode;
		il_servo_state_t state;
		int32_t pos_act;
		int32_t vel_act;
		int16_t torque_tgt;
		int16_t torque_act;
		int32_t pos_tgt;
		int32_t vel_tgt;
	} store;
} il_virtual_servo_t;

/** Obtain VIRTUAL servo from parent. */
#define to_virtual_servo(ptr) container_of(ptr, struct il_virtual_servo, servo)

#endif
