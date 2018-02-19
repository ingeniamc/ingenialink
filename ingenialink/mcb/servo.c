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

#include "servo.h"

#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
}

/**
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_mcb_servo_t *).
 */
static void servo_destroy(void *ctx)
{
	il_mcb_servo_t *this = ctx;

	il_servo_base__deinit(&this->servo);

	free(this);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_mcb_servo__retain(il_servo_t *servo)
{
	il_mcb_servo_t *this = to_mcb_servo(servo);

	il_utils__refcnt_retain(this->refcnt);
}

void il_mcb_servo__release(il_servo_t *servo)
{
	il_mcb_servo_t *this = to_mcb_servo(servo);

	il_utils__refcnt_release(this->refcnt);
}

/**
 * Decode the PDS state.
 *
 * @param [in] sw
 *	Statusword value.
 *
 * @return
 *	PDS state (IL_SERVO_STATE_NRDY if unknown).
 */
void il_mcb_servo__state_decode(uint16_t sw, il_servo_state_t *state,
				int *flags)
{
	(void)sw;
	(void)state;
	(void)flags;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

static il_servo_t *il_mcb_servo_create(il_net_t *net, uint16_t id,
				       const char *dict)
{
	int r;

	il_mcb_servo_t *this;

	/* allocate servo */
	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}

	r = il_servo_base__init(&this->servo, net, id, dict);
	if (r < 0)
		goto cleanup_servo;

	this->servo.ops = &il_mcb_servo_ops;

	/* initialize, setup refcnt */
	this->refcnt = il_utils__refcnt_create(servo_destroy, this);
	if (!this->refcnt)
		goto cleanup_base;

	return &this->servo;

cleanup_base:
	il_servo_base__deinit(&this->servo);

cleanup_servo:
	free(this);

	return NULL;
}

static void il_mcb_servo_destroy(il_servo_t *servo)
{
	il_mcb_servo_t *this = to_mcb_servo(servo);

	il_utils__refcnt_release(this->refcnt);
}

static int il_mcb_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	(void)servo;

	strncpy(name, "Drive", sz);

	return 0;
}

static int il_mcb_servo_name_set(il_servo_t *servo, const char *name)
{
	(void)servo;
	(void)name;

	return not_supported();
}

static int il_mcb_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
{
	(void)servo;

	info->serial = 0;
	strncpy(info->name, "Drive", sizeof(info->name) - 1);
	strncpy(info->sw_version, "1.0.0", sizeof(info->sw_version) - 1);
	strncpy(info->hw_variant, "A", sizeof(info->hw_variant) - 1);
	info->prod_code = 0;
	info->revision = 0;

	return 0;
}

static int il_mcb_servo_store_all(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_store_comm(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_store_app(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_units_update(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static double il_mcb_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	(void)servo;
	(void)reg;

	return not_supported();
}

static int il_mcb_servo_disable(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_switch_on(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return not_supported();
}

static int il_mcb_servo_enable(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return not_supported();
}

static int il_mcb_servo_fault_reset(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	(void)servo;
	(void)mode;

	return not_supported();
}

static int il_mcb_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	(void)servo;
	(void)mode;

	return not_supported();
}

static int il_mcb_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_mcb_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_mcb_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_mcb_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_mcb_servo_homing_start(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_mcb_servo_homing_wait(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return not_supported();
}

static int il_mcb_servo_torque_get(il_servo_t *servo, double *torque)
{
	(void)servo;
	(void)torque;

	return not_supported();
}

static int il_mcb_servo_torque_set(il_servo_t *servo, double torque)
{
	(void)servo;
	(void)torque;

	return not_supported();
}

static int il_mcb_servo_position_get(il_servo_t *servo, double *pos)
{
	(void)servo;
	(void)pos;

	return not_supported();
}

static int il_mcb_servo_position_set(il_servo_t *servo, double pos,
				     int immediate, int relative,
				     int sp_timeout)
{
	(void)servo;
	(void)pos;
	(void)immediate;
	(void)relative;
	(void)sp_timeout;

	return not_supported();
}

static int il_mcb_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_mcb_servo_velocity_get(il_servo_t *servo, double *vel)
{
	(void)servo;
	(void)vel;

	return not_supported();
}

static int il_mcb_servo_velocity_set(il_servo_t *servo, double vel)
{
	(void)servo;
	(void)vel;

	return not_supported();
}

static int il_mcb_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_mcb_servo_wait_reached(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return not_supported();
}

/** E-USB servo operations. */
const il_servo_ops_t il_mcb_servo_ops = {
	/* internal */
	il_mcb_servo__retain,
	il_mcb_servo__release,
	il_mcb_servo__state_decode,
	/* public */
	il_mcb_servo_create,
	il_mcb_servo_destroy,
	il_servo_base__state_get,
	il_servo_base__state_subscribe,
	il_servo_base__state_unsubscribe,
	il_servo_base__emcy_subscribe,
	il_servo_base__emcy_unsubscribe,
	il_servo_base__dict_get,
	il_servo_base__dict_load,
	il_mcb_servo_name_get,
	il_mcb_servo_name_set,
	il_mcb_servo_info_get,
	il_mcb_servo_store_all,
	il_mcb_servo_store_comm,
	il_mcb_servo_store_app,
	il_mcb_servo_units_update,
	il_mcb_servo_units_factor,
	il_servo_base__units_torque_get,
	il_servo_base__units_torque_set,
	il_servo_base__units_pos_get,
	il_servo_base__units_pos_set,
	il_servo_base__units_vel_get,
	il_servo_base__units_vel_set,
	il_servo_base__units_acc_get,
	il_servo_base__units_acc_set,
	il_servo_base__raw_read_u8,
	il_servo_base__raw_read_s8,
	il_servo_base__raw_read_u16,
	il_servo_base__raw_read_s16,
	il_servo_base__raw_read_u32,
	il_servo_base__raw_read_s32,
	il_servo_base__raw_read_u64,
	il_servo_base__raw_read_s64,
	il_servo_base__raw_read_float,
	il_servo_base__read,
	il_servo_base__raw_write_u8,
	il_servo_base__raw_write_s8,
	il_servo_base__raw_write_u16,
	il_servo_base__raw_write_s16,
	il_servo_base__raw_write_u32,
	il_servo_base__raw_write_s32,
	il_servo_base__raw_write_u64,
	il_servo_base__raw_write_s64,
	il_servo_base__raw_write_float,
	il_servo_base__write,
	il_mcb_servo_disable,
	il_mcb_servo_switch_on,
	il_mcb_servo_enable,
	il_mcb_servo_fault_reset,
	il_mcb_servo_mode_get,
	il_mcb_servo_mode_set,
	il_mcb_servo_ol_voltage_get,
	il_mcb_servo_ol_voltage_set,
	il_mcb_servo_ol_frequency_get,
	il_mcb_servo_ol_frequency_set,
	il_mcb_servo_homing_start,
	il_mcb_servo_homing_wait,
	il_mcb_servo_torque_get,
	il_mcb_servo_torque_set,
	il_mcb_servo_position_get,
	il_mcb_servo_position_set,
	il_mcb_servo_position_res_get,
	il_mcb_servo_velocity_get,
	il_mcb_servo_velocity_set,
	il_mcb_servo_velocity_res_get,
	il_mcb_servo_wait_reached,
};
