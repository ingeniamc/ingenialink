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
#include "ingenialink/base/servo.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
}

/**
 * Randomize a value by randomizing its amplitude by a factor.
 *
 * @return
 *	Random factor (1, 1 + RANDOM_FACTOR).
 */
static double randomize(void)
{
	return 1 + (double)rand() * RANDOM_FACTOR / RAND_MAX;
}

/**
 * Obtain register (pre-defined or from dictionary).
 *
 * @param [in] dict
 *	Dictionary.
 * @param [in] reg_pdef
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [out] reg
 *	Where register will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int get_reg(il_dict_t *dict, const il_reg_t *reg_pdef,
		   const char *id, const il_reg_t **reg)
{
	int r;

	/* obtain register (predefined or from dictionary) */
	if (reg_pdef) {
		*reg = reg_pdef;
	} else {
		if (!dict) {
			ilerr__set("No dictionary loaded");
			return IL_EFAIL;
		}

		r = il_dict_reg_get(dict, id, reg, DFLT_SUBNODE);
		if (r < 0)
			return r;
	}

	return 0;
}

/**
 * Raw read.
 *
 * @param [in] servo
 *	Servo.
 * @param [in] reg_pdef
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] dtype
 *	Expected data type.
 * @param [out] buf
 *	Where data will be stored.
 * @param [in] sz
 *	Buffer size.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int raw_read(il_servo_t *servo, const il_reg_t *reg_pdef,
		    const char *id, il_reg_dtype_t dtype, void *buf, size_t sz)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	int r;
	const il_reg_t *reg;

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg_pdef, id, &reg);
	if (r < 0)
		return r;

	/* verify register properties */
	if (reg->dtype != dtype) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_WO) {
		ilerr__set("Register is write-only");
		return IL_EACCESS;
	}

	/* TODO: this should use a hash table */
	if (reg->address == ADDR_POS_ACT) {
		int32_t pos_act = this->store.pos_act;

		pos_act = (int32_t)((double)pos_act * randomize());
		memcpy(buf, &pos_act, sz);
	} else if (reg->address == ADDR_VEL_ACT) {
		int32_t vel_act = this->store.vel_act;

		vel_act = (int32_t)((double)vel_act * randomize());
		memcpy(buf, &vel_act, sz);
	} else if (reg->address == ADDR_TORQUE_TGT) {
		memcpy(buf, &this->store.torque_tgt, sz);
	} else if (reg->address == ADDR_TORQUE_ACT) {
		int16_t torque_act = this->store.torque_act;

		torque_act = (int16_t)((double)torque_act * randomize());
		memcpy(buf, &torque_act, sz);
	} else if (reg->address == ADDR_POS_TGT) {
		memcpy(buf, &this->store.pos_tgt, sz);
	} else if (reg->address == ADDR_VEL_TGT) {
		memcpy(buf, &this->store.vel_tgt, sz);
	} else if (reg->address == ADDR_TEMP_ACT) {
		int32_t temp_act = (int32_t)(TEMPERATURE * randomize());

		memcpy(buf, &temp_act, sz);
	} else if (reg->address == ADDR_DC_VOLTAGE) {
		uint32_t dc_voltage = (uint32_t)(DC_VOLTAGE * randomize());

		memcpy(buf, &dc_voltage, sz);
	} else {
		/* fake read */
		memset(buf, 0, sz);
	}

	return 0;
}

/**
 * Raw write.
 *
 * @param [in] servo
 *	Servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] dtype
 *	Expected data type.
 * @param [in] data
 *	Data buffer.
 * @param [in] sz
 *	Data buffer size.
 * @param [in] confirmed
 *	Confirm write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int raw_write(il_servo_t *servo, const il_reg_t *reg,
		     il_reg_dtype_t dtype, const void *data, size_t sz,
		     int confirmed)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	(void)sz;
	(void)confirmed;

	/* verify register properties */
	if (reg->dtype != dtype) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	/* TODO: this should use a hash table */
	if (reg->address == ADDR_TORQUE_TGT) {
		memcpy(&this->store.torque_tgt, data,
		       sizeof(this->store.torque_tgt));
	} else if (reg->address == ADDR_POS_TGT) {
		memcpy(&this->store.pos_tgt, data, sizeof(this->store.pos_tgt));
	} else if (reg->address == ADDR_VEL_TGT) {
		memcpy(&this->store.vel_tgt, data, sizeof(this->store.vel_tgt));
	}

	return 0;
}

/**
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_virtual_servo_t *).
 */
static void servo_destroy(void *ctx)
{
	il_virtual_servo_t *this = ctx;

	il_servo_base__deinit(&this->servo);

	free(this);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_virtual_servo__retain(il_servo_t *servo)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	il_utils__refcnt_retain(this->refcnt);
}

void il_virtual_servo__release(il_servo_t *servo)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

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
void il_virtual_servo__state_decode(uint16_t sw, il_servo_state_t *state,
				    int *flags)
{
	(void)sw;
	(void)state;
	(void)flags;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

static il_servo_t *il_virtual_servo_create(il_net_t *net, uint16_t id,
					   const char *dict)
{
	int r;

	il_virtual_servo_t *this;

	/* allocate servo */
	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}

	r = il_servo_base__init(&this->servo, net, id, dict);
	if (r < 0)
		goto cleanup_servo;

	this->servo.ops = &il_virtual_servo_ops;

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

static void il_virtual_servo_destroy(il_servo_t *servo)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	il_utils__refcnt_release(this->refcnt);
}

static int il_virtual_servo_reset(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

void il_servo_virtual_state_get(il_servo_t *servo, il_servo_state_t *state,
				int *flags)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	*state = this->store.state;
	*flags = 0;
}

static int il_virtual_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	(void)servo;

	strncpy(name, "Virtual", sz);

	return 0;
}

static int il_virtual_servo_name_set(il_servo_t *servo, const char *name)
{
	(void)servo;
	(void)name;

	return not_supported();
}

static int il_virtual_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
{
	(void)servo;

	info->serial = 0x00001714;
	strncpy(info->name, "Virtual", sizeof(info->name) - 1);
	strncpy(info->sw_version, "1.0.0", sizeof(info->sw_version) - 1);
	strncpy(info->hw_variant, "OCT", sizeof(info->hw_variant) - 1);
	info->prod_code = 1714;
	info->revision = 1;

	return 0;
}

static int il_virtual_servo_store_all(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_virtual_servo_store_comm(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_virtual_servo_store_app(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_virtual_servo_units_update(il_servo_t *servo)
{
	(void)servo;

	return 0;
}

static double il_virtual_servo_units_factor(il_servo_t *servo,
					    const il_reg_t *reg)
{
	(void)servo;
	(void)reg;

	return 1.0;
}

int il_servo_virtual_raw_read_u8(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint8_t *buf)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_U8, buf, sizeof(*buf));
}

int il_servo_virtual_raw_read_s8(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int8_t *buf)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_S8, buf, sizeof(*buf));
}

int il_servo_virtual_raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, uint16_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U16, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_be_16(*buf);

	return r;
}

int il_servo_virtual_raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, int16_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S16, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int16_t)__swap_be_16(*buf);

	return r;
}

int il_servo_virtual_raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, uint32_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U32, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_be_32(*buf);

	return r;
}

int il_servo_virtual_raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, int32_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S32, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int32_t)__swap_be_32(*buf);

	return r;
}

int il_servo_virtual_raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, uint64_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U64, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_be_64(*buf);

	return r;
}

int il_servo_virtual_raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, int64_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S64, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int64_t)__swap_be_64(*buf);

	return r;
}

int il_servo_virtual_raw_read_float(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, float *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_FLOAT, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_be_float(*buf);

	return r;
}

int il_servo_virtual_raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, uint8_t val, int confirm)
{
	int r;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.u8) || (val > reg->range.max.u8)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	return raw_write(servo, reg_, IL_REG_DTYPE_U8, &val, sizeof(val),
			 confirm);
}

int il_servo_virtual_raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, int8_t val, int confirm)
{
	int r;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.s8) || (val > reg->range.max.s8)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	return raw_write(servo, reg_, IL_REG_DTYPE_S8, &val, sizeof(val),
			 confirm);
}

int il_servo_virtual_raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, uint16_t val, int confirm)
{
	int r;
	uint16_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.u16) || (val > reg->range.max.u16)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = __swap_be_16(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U16, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, int16_t val, int confirm)
{
	int r;
	int16_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.s16) || (val > reg->range.max.s16)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = (int16_t)__swap_be_16(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S16, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, uint32_t val, int confirm)
{
	int r;
	uint32_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.u32) || (val > reg->range.max.u32)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = __swap_be_32(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U32, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, int32_t val, int confirm)
{
	int r;
	int32_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.s32) || (val > reg->range.max.s32)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = (int32_t)__swap_be_32(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S32, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, uint64_t val, int confirm)
{
	int r;
	uint64_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.u64) || (val > reg->range.max.u64)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = __swap_be_64(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U64, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, int64_t val, int confirm)
{
	int r;
	int64_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	if ((val < reg->range.min.s64) || (val > reg->range.max.s64)) {
		ilerr__set("Value out of range");
		return IL_EINVAL;
	}

	val_ = (int64_t)__swap_be_64(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S64, &val_, sizeof(val_),
			 confirm);
}

int il_servo_virtual_raw_write_float(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, float val, int confirm)
{
	int r;
	float val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	val_ = __swap_be_float(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_FLOAT, &val_,
			 sizeof(val_), confirm);
}

static int il_virtual_servo_disable(il_servo_t *servo, uint8_t subnode, int timeout)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	this->store.state = IL_SERVO_STATE_DISABLED;

	return 0;
}

static int il_virtual_servo_switch_on(il_servo_t *servo, int timeout)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	(void)timeout;

	this->store.state = IL_SERVO_STATE_ON;

	return 0;
}

static int il_virtual_servo_enable(il_servo_t *servo, uint8_t subnode, int timeout)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	(void)timeout;

	this->store.state = IL_SERVO_STATE_ENABLED;

	return 0;
}

static int il_virtual_servo_fault_reset(il_servo_t *servo, uint8_t subnode, int timeout)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	this->store.state = IL_SERVO_STATE_DISABLED;

	return 0;
}

static int il_virtual_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	*mode = this->store.mode;

	return 0;
}

static int il_virtual_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	this->store.mode = mode;

	return 0;
}

static int il_virtual_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_virtual_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_virtual_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_virtual_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_virtual_servo_homing_start(il_servo_t *servo)
{
	return il_servo_position_set(servo, 0, 0, 0, 0);
}

static int il_virtual_servo_homing_wait(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return 0;
}

static int il_virtual_servo_torque_get(il_servo_t *servo, double *torque)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	*torque = (double)this->store.torque_act * randomize();

	return 0;
}

static int il_virtual_servo_torque_set(il_servo_t *servo, double torque)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	this->store.torque_act = (int32_t)torque;
	this->store.torque_tgt = (int32_t)torque;

	return 0;
}

static int il_virtual_servo_position_get(il_servo_t *servo, double *pos)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	*pos = (double)this->store.pos_act * randomize();

	return 0;
}

static int il_virtual_servo_position_set(il_servo_t *servo, double pos,
					 int immediate, int relative,
					 int sp_timeout)
{
	(void)immediate;
	(void)sp_timeout;

	il_virtual_servo_t *this = to_virtual_servo(servo);

	if (relative)
		pos += (double)this->store.pos_act;

	this->store.pos_act = (int32_t)pos;
	this->store.pos_tgt = (int32_t)pos;

	return 0;
}

static int il_virtual_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_virtual_servo_velocity_get(il_servo_t *servo, double *vel)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	*vel = (double)this->store.vel_act * randomize();

	return 0;
}

static int il_virtual_servo_velocity_set(il_servo_t *servo, double vel)
{
	il_virtual_servo_t *this = to_virtual_servo(servo);

	this->store.vel_act = (int32_t)vel;
	this->store.vel_tgt = (int32_t)vel;

	return 0;
}

static int il_virtual_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_virtual_servo_wait_reached(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return 0;
}

/** E-USB servo operations. */
const il_servo_ops_t il_virtual_servo_ops = {
	/* internal */
	._retain = il_virtual_servo__retain,
	._release = il_virtual_servo__release,
	._state_decode = il_virtual_servo__state_decode,
	/* public */
	.create = il_virtual_servo_create,
	.destroy = il_virtual_servo_destroy,
	.reset = il_virtual_servo_reset,
	.state_get = il_servo_virtual_state_get,
	.state_subscribe = il_servo_base__state_subscribe,
	.state_unsubscribe = il_servo_base__state_unsubscribe,
	.emcy_subscribe = il_servo_base__emcy_subscribe,
	.emcy_unsubscribe = il_servo_base__emcy_unsubscribe,
	.dict_get = il_servo_base__dict_get,
	.dict_load = il_servo_base__dict_load,
	.name_get = il_virtual_servo_name_get,
	.name_set = il_virtual_servo_name_set,
	.info_get = il_virtual_servo_info_get,
	.store_all = il_virtual_servo_store_all,
	.store_comm = il_virtual_servo_store_comm,
	.store_app = il_virtual_servo_store_app,
	.units_update = il_virtual_servo_units_update,
	.units_factor = il_virtual_servo_units_factor,
	.units_torque_get = il_servo_base__units_torque_get,
	.units_torque_set = il_servo_base__units_torque_set,
	.units_pos_get = il_servo_base__units_pos_get,
	.units_pos_set = il_servo_base__units_pos_set,
	.units_vel_get = il_servo_base__units_vel_get,
	.units_vel_set = il_servo_base__units_vel_set,
	.units_acc_get = il_servo_base__units_acc_get,
	.units_acc_set = il_servo_base__units_acc_set,
	.raw_read_u8 = il_servo_virtual_raw_read_u8,
	.raw_read_s8 = il_servo_virtual_raw_read_s8,
	.raw_read_u16 = il_servo_virtual_raw_read_u16,
	.raw_read_s16 = il_servo_virtual_raw_read_s16,
	.raw_read_u32 = il_servo_virtual_raw_read_u32,
	.raw_read_s32 = il_servo_virtual_raw_read_s32,
	.raw_read_u64 = il_servo_virtual_raw_read_u64,
	.raw_read_s64 = il_servo_virtual_raw_read_s64,
	.raw_read_float = il_servo_virtual_raw_read_float,
	.read = il_servo_base__read,
	.raw_write_u8 = il_servo_virtual_raw_write_u8,
	.raw_write_s8 = il_servo_virtual_raw_write_s8,
	.raw_write_u16 = il_servo_virtual_raw_write_u16,
	.raw_write_s16 = il_servo_virtual_raw_write_s16,
	.raw_write_u32 = il_servo_virtual_raw_write_u32,
	.raw_write_s32 = il_servo_virtual_raw_write_s32,
	.raw_write_u64 = il_servo_virtual_raw_write_u64,
	.raw_write_s64 = il_servo_virtual_raw_write_s64,
	.raw_write_float = il_servo_virtual_raw_write_float,
	.write = il_servo_base__write,
	.disable = il_virtual_servo_disable,
	.switch_on = il_virtual_servo_switch_on,
	.enable = il_virtual_servo_enable,
	.fault_reset = il_virtual_servo_fault_reset,
	.mode_get = il_virtual_servo_mode_get,
	.mode_set = il_virtual_servo_mode_set,
	.ol_voltage_get = il_virtual_servo_ol_voltage_get,
	.ol_voltage_set = il_virtual_servo_ol_voltage_set,
	.ol_frequency_get = il_virtual_servo_ol_frequency_get,
	.ol_frequency_set = il_virtual_servo_ol_frequency_set,
	.homing_start = il_virtual_servo_homing_start,
	.homing_wait = il_virtual_servo_homing_wait,
	.torque_get = il_virtual_servo_torque_get,
	.torque_set = il_virtual_servo_torque_set,
	.position_get = il_virtual_servo_position_get,
	.position_set = il_virtual_servo_position_set,
	.position_res_get = il_virtual_servo_position_res_get,
	.velocity_get = il_virtual_servo_velocity_get,
	.velocity_set = il_virtual_servo_velocity_set,
	.velocity_res_get = il_virtual_servo_velocity_res_get,
	.wait_reached = il_virtual_servo_wait_reached,
};
