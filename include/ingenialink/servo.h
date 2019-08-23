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

#ifndef INGENIALINK_SERVO_H_
#define INGENIALINK_SERVO_H_

#include "public/ingenialink/servo.h"

/**
 * Retain a reference of the servo.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 */
void il_servo__retain(il_servo_t *servo);

/**
 * Release a reference of the servo.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 */
void il_servo__release(il_servo_t *servo);

/**
 * Decode the PDS state.
 *
 * @param [in] sw
 *	Statusword value.
 * @param [out] state
 *	State
 * @param [out] flags
 *	Flags
 */
void il_servo__state_decode(uint16_t sw, il_servo_state_t *state, int *flags);

/** Servo operations. */
typedef struct {
	/* internal */
	void (*_retain)(il_servo_t *servo);
	void (*_release)(il_servo_t *servo);
	void (*_state_decode)(uint16_t sw, il_servo_state_t *state, int *flags);
	/* public */
	il_servo_t *(*create)(il_net_t *net, uint16_t id, const char *dict);
	void (*destroy)(il_servo_t *servo);
	int (*reset)(il_servo_t *servo);
	void (*state_get)(
		il_servo_t *servo, il_servo_state_t *state, int *flags);
	int (*state_subscribe)(
		il_servo_t *servo, il_servo_state_subscriber_cb_t cb,
		void *ctx);
	void (*state_unsubscribe)(il_servo_t *servo, int slot);
	int (*emcy_subscribe)(
		il_servo_t *servo, il_servo_emcy_subscriber_cb_t cb, void *ctx);
	void (*emcy_unsubscribe)(il_servo_t *servo, int slot);
	il_dict_t *(*dict_get)(il_servo_t *servo);
	int (*dict_load)(il_servo_t *servo, const char *dict);
	int (*name_get)(il_servo_t *servo, char *name, size_t sz);
	int (*name_set)(il_servo_t *servo, const char *name);
	int (*info_get)(il_servo_t *servo, il_servo_info_t *info);
	int (*store_all)(il_servo_t *servo);
	int (*store_comm)(il_servo_t *servo);
	int (*store_app)(il_servo_t *servo);
	int (*units_update)(il_servo_t *servo);
	double (*units_factor)(il_servo_t *servo, const il_reg_t *reg);
	il_units_torque_t (*units_torque_get)(il_servo_t *servo);
	void (*units_torque_set)(il_servo_t *servo, il_units_torque_t units);
	il_units_pos_t (*units_pos_get)(il_servo_t *servo);
	void (*units_pos_set)(il_servo_t *servo, il_units_pos_t units);
	il_units_vel_t (*units_vel_get)(il_servo_t *servo);
	void (*units_vel_set)(il_servo_t *servo, il_units_vel_t units);
	il_units_acc_t (*units_acc_get)(il_servo_t *servo);
	void (*units_acc_set)(il_servo_t *servo, il_units_acc_t units);
	int (*raw_read_u8)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint8_t *buf);
	int (*raw_read_s8)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int8_t *buf);
	int (*raw_read_u16)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint16_t *buf);
	int (*raw_read_s16)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int16_t *buf);
	int (*raw_read_u32)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint32_t *buf);
	int (*raw_read_str)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint32_t *buf);
	int (*raw_read_s32)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int32_t *buf);
	int (*raw_read_u64)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint64_t *buf);
	int (*raw_read_s64)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int64_t *buf);
	int (*raw_read_float)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		float *buf);
	int (*read)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		double *buf);
	int (*raw_write_u8)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint8_t val, int confirm);
	int (*raw_write_s8)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int8_t val, int confirm);
	int (*raw_write_u16)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint16_t val, int confirm);
	int (*raw_write_s16)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int16_t val, int confirm);
	int (*raw_write_u32)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint32_t val, int confirm);
	int (*raw_wait_write_u32)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint32_t val, int confirm);
	int (*raw_write_s32)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int32_t val, int confirm);
	int (*raw_write_u64)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		uint64_t val, int confirm);
	int (*raw_write_s64)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		int64_t val, int confirm);
	int (*raw_write_float)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		float val, int confirm);
	int (*write)(
		il_servo_t *servo, const il_reg_t *reg, const char *id,
		double val, int confirm);
	int (*disable)(il_servo_t *servo);
	int (*switch_on)(il_servo_t *servo, int timeout);
	int (*enable)(il_servo_t *servo, int timeout);
	int (*fault_reset)(il_servo_t *servo);
	int (*mode_get)(il_servo_t *servo, il_servo_mode_t *mode);
	int (*mode_set)(il_servo_t *servo, il_servo_mode_t mode);
	int (*ol_voltage_get)(il_servo_t *servo, double *voltage);
	int (*ol_voltage_set)(il_servo_t *servo, double voltage);
	int (*ol_frequency_get)(il_servo_t *servo, double *freq);
	int (*ol_frequency_set)(il_servo_t *servo, double freq);
	int (*homing_start)(il_servo_t *servo);
	int (*homing_wait)(il_servo_t *servo, int timeout);
	int (*torque_get)(il_servo_t *servo, double *torque);
	int (*torque_set)(il_servo_t *servo, double torque);
	int (*position_get)(il_servo_t *servo, double *pos);
	int (*position_set)(
		il_servo_t *servo, double pos, int immediate, int relative,
		int sp_timeout);
	int (*position_res_get)(il_servo_t *servo, uint32_t *res);
	int (*velocity_get)(il_servo_t *servo, double *vel);
	int (*velocity_set)(il_servo_t *servo, double vel);
	int (*velocity_res_get)(il_servo_t *servo, uint32_t *res);
	int (*wait_reached)(il_servo_t *servo, int timeout);
} il_servo_ops_t;

#endif

