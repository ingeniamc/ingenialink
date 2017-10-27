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

#include "servo.h"
#include "mc.h"

#include <assert.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "public/ingenialink/const.h"
#include "public/ingenialink/registers.h"
#include "ingenialink/err.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Statusword update callback
 *
 * @param [in] ctx
 *	Context (servo_t *).
 * @param [in] sw
 *	Statusword value.
 */
static void sw_update(void *ctx, uint16_t sw)
{
	il_servo_t *servo = ctx;

	osal_mutex_lock(servo->sw.lock);

	if (servo->sw.value != sw) {
		servo->sw.value = sw;
		osal_cond_broadcast(servo->sw.changed);
	}

	osal_mutex_unlock(servo->sw.lock);
}

/**
 * Obtain the current statusword value.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Statusword value.
 */
static uint16_t sw_get(il_servo_t *servo)
{
	uint16_t sw;

	osal_mutex_lock(servo->sw.lock);
	sw = servo->sw.value;
	osal_mutex_unlock(servo->sw.lock);

	return sw;
}

/**
 * Wait until the statusword changes its value.
 *
 * @note
 *	The timeout is not an absolute timeout, but an interval timeout.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] sw
 *	Current statusword value.
 * @param [in] timeout
 *	Timeout (ms).
 */
static int sw_wait_change(il_servo_t *servo, uint16_t sw, int timeout)
{
	int r = 0;

	osal_mutex_lock(servo->sw.lock);

	if (servo->sw.value == sw) {
		r = osal_cond_wait(servo->sw.changed, servo->sw.lock, timeout);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
		} else if (r < 0) {
			ilerr__set("Statusword wait change failed");
			r = IL_EFAIL;
		}
	}

	osal_mutex_unlock(servo->sw.lock);

	return r;
}

/**
 * Wait until the statusword has the requested value
 *
 * @note
 *	The timeout is not an absolute timeout, but an interval timeout.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] msk
 *	Statusword mask.
 * @param [in] val
 *	Statusword value.
 * @param [in] timeout
 *	Timeout (ms).
 */
static int sw_wait_value(il_servo_t *servo, uint16_t msk, uint16_t val,
			 int timeout)
{
	int r = 0;
	uint16_t result;

	/* wait until the flag changes to the requested state */
	osal_mutex_lock(servo->sw.lock);

	do {
		result = servo->sw.value & msk;
		if (result != val) {
			r = osal_cond_wait(servo->sw.changed, servo->sw.lock,
					   timeout);
			if (r == OSAL_ETIMEDOUT) {
				ilerr__set("Operation timed out");
				r = IL_ETIMEDOUT;
			} else if (r < 0) {
				ilerr__set("Statusword wait change failed");
				r = IL_EFAIL;
			}
		}
	} while ((result != val) && (r == 0));

	osal_mutex_unlock(servo->sw.lock);

	return r;
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
static il_servo_state_t pds_state_decode(uint16_t sw)
{
	if ((sw & IL_MC_PDS_STA_NRTSO_MSK) == IL_MC_PDS_STA_NRTSO)
		return IL_SERVO_STATE_NRDY;
	else if ((sw & IL_MC_PDS_STA_SOD_MSK) == IL_MC_PDS_STA_SOD)
		return IL_SERVO_STATE_DISABLED;
	else if ((sw & IL_MC_PDS_STA_RTSO_MSK) == IL_MC_PDS_STA_RTSO)
		return IL_SERVO_STATE_RDY;
	else if ((sw & IL_MC_PDS_STA_SO_MSK) == IL_MC_PDS_STA_SO)
		return IL_SERVO_STATE_ON;
	else if ((sw & IL_MC_PDS_STA_OE_MSK) == IL_MC_PDS_STA_OE)
		return IL_SERVO_STATE_ENABLED;
	else if ((sw & IL_MC_PDS_STA_QSA_MSK) == IL_MC_PDS_STA_QSA)
		return IL_SERVO_STATE_QSTOP;
	else if ((sw & IL_MC_PDS_STA_FRA_MSK) == IL_MC_PDS_STA_FRA)
		return IL_SERVO_STATE_FAULTR;
	else if ((sw & IL_MC_PDS_STA_F_MSK) == IL_MC_PDS_STA_F)
		return IL_SERVO_STATE_FAULT;

	return IL_SERVO_STATE_NRDY;
}

/**
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_servo_t *).
 */
void servo_destroy(void *ctx)
{
	il_servo_t *servo = ctx;

	il_net__sw_unsubscribe(servo->net, servo->sw.slot);
	il_net__release(servo->net);

	osal_cond_destroy(servo->sw.changed);
	osal_mutex_destroy(servo->sw.lock);

	osal_mutex_destroy(servo->units.lock);

	free(servo);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_servo__retain(il_servo_t *servo)
{
	refcnt__retain(servo->refcnt);
}

void il_servo__release(il_servo_t *servo)
{
	refcnt__release(servo->refcnt);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_servo_t *il_servo_create(il_net_t *net, uint8_t id, int timeout)
{
	int r;

	il_servo_t *servo;
	uint16_t sw;

	assert(net);

	/* validate node id */
	if ((id < SERVOID_MIN) || (id > SERVOID_MAX)) {
		ilerr__set("Servo id out of range");
		return NULL;
	}

	/* allocate servo */
	servo = malloc(sizeof(*servo));
	if (!servo) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}

	servo->net = net;
	il_net__retain(servo->net);
	servo->id = id;
	servo->timeout = timeout;

	/* setup refcnt */
	servo->refcnt = refcnt__create(servo_destroy, servo);
	if (!servo->refcnt)
		goto cleanup_servo;

	/* configure units */
	servo->units.lock = osal_mutex_create();
	if (!servo->units.lock) {
		ilerr__set("Units lock allocation failed");
		goto cleanup_refcnt;
	}

	r = il_servo_units_update(servo);
	if (r < 0)
		goto cleanup_units_lock;

	servo->units.torque = IL_UNITS_TORQUE_NATIVE;
	servo->units.pos = IL_UNITS_POS_NATIVE;
	servo->units.vel = IL_UNITS_VEL_NATIVE;
	servo->units.acc = IL_UNITS_ACC_NATIVE;

	/* configure statusword subscription */
	servo->sw.lock = osal_mutex_create();
	if (!servo->sw.lock) {
		ilerr__set("Statusword subscriber lock allocation failed");
		goto cleanup_units_lock;
	}

	servo->sw.changed = osal_cond_create();
	if (!servo->sw.changed) {
		ilerr__set("Statusword subscriber condition allocation failed");
		goto cleanup_sw_lock;
	}

	servo->sw.value = 0;

	r = il_net__sw_subscribe(servo->net, servo->id, sw_update, servo);
	if (r < 0)
		goto cleanup_sw_changed;

	servo->sw.slot = r;

	/* trigger update (with manual read) */
	(void)il_servo_raw_read_u16(servo, &IL_REG_STS_WORD, &sw);

	return servo;

cleanup_sw_changed:
	osal_cond_destroy(servo->sw.changed);

cleanup_sw_lock:
	osal_mutex_destroy(servo->sw.lock);

cleanup_units_lock:
	osal_mutex_destroy(servo->units.lock);

cleanup_refcnt:
	refcnt__destroy(servo->refcnt);

cleanup_servo:
	il_net__release(servo->net);
	free(servo);

	return NULL;
}

void il_servo_destroy(il_servo_t *servo)
{
	assert(servo);

	refcnt__release(servo->refcnt);
}

int il_servo_emcy_subscribe(il_servo_t *servo, il_servo_emcy_subscriber_cb_t cb,
			    void *ctx)
{
	assert(servo);
	assert(cb);

	return il_net__emcy_subscribe(servo->net, servo->id,
				      (il_net_emcy_subscriber_cb_t)cb, ctx);
}

void il_servo_emcy_unsubscribe(il_servo_t *servo, int slot)
{
	assert(servo);

	il_net__emcy_unsubscribe(servo->net, slot);
}

int il_servo_units_update(il_servo_t *servo)
{
	int r;
	uint32_t rated_torque, pos_res, vel_res, ppitch;

	assert(servo);

	r = il_servo_raw_read_u32(servo, &IL_REG_RATED_TORQUE, &rated_torque);
	if (r < 0)
		return r;

	r = il_servo_position_res_get(servo, &pos_res);
	if (r < 0)
		return r;

	r = il_servo_velocity_res_get(servo, &vel_res);
	if (r < 0)
		return r;

	r = il_servo_raw_read_u32(servo, &IL_REG_MOTPARAM_PPITCH, &ppitch);
	if (r < 0)
		return r;

	servo->cfg.rated_torque = (double)rated_torque;
	servo->cfg.pos_res = (double)pos_res;
	servo->cfg.vel_res = (double)vel_res;
	servo->cfg.acc_res = servo->cfg.pos_res;
	servo->cfg.ppitch = (double)ppitch / 1000000;

	return 0;
}

double il_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	double factor;

	assert(servo);
	assert(reg);

	osal_mutex_lock(servo->units.lock);

	switch (reg->phy) {
	case IL_REG_PHY_TORQUE:
		switch (servo->units.torque) {
		case IL_UNITS_TORQUE_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_TORQUE_MN:
			factor = servo->cfg.rated_torque / 1000000.;
			break;
		case IL_UNITS_TORQUE_N:
			factor = servo->cfg.rated_torque / 1000.;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_POS:
		switch (servo->units.pos) {
		case IL_UNITS_POS_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_POS_REV:
			factor = 1. / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_RAD:
			factor = 2. * M_PI / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_DEG:
			factor = 360. / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_UM:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_MM:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_M:
			factor = 1. * servo->cfg.ppitch / servo->cfg.pos_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_VEL:
		switch (servo->units.vel) {
		case IL_UNITS_VEL_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_VEL_RPS:
			factor = 1. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RPM:
			factor = 60. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RAD_S:
			factor = 2. * M_PI / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_DEG_S:
			factor = 360. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_UM_S:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_MM_S:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_M_S:
			factor = 1. * servo->cfg.ppitch / servo->cfg.vel_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_ACC:
		switch (servo->units.acc) {
		case IL_UNITS_ACC_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_ACC_REV_S2:
			factor = 1. / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_RAD_S2:
			factor = 2. * M_PI / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_DEG_S2:
			factor = 360. / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_UM_S2:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_MM_S2:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_M_S2:
			factor = 1. * servo->cfg.ppitch / servo->cfg.acc_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_VOLT_REL:
		factor = 1. / VOLT_REL_RANGE;
		break;
	case IL_REG_PHY_RAD:
		factor = 2. * M_PI / RAD_RANGE;
		break;
	default:
		factor = 1.;
		break;
	}

	osal_mutex_unlock(servo->units.lock);

	return factor;
}

il_units_torque_t il_servo_units_torque_get(il_servo_t *servo)
{
	il_units_torque_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.torque;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_torque_set(il_servo_t *servo, il_units_torque_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.torque = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_pos_t il_servo_units_pos_get(il_servo_t *servo)
{
	il_units_pos_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.pos;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_pos_set(il_servo_t *servo, il_units_pos_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.pos = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_vel_t il_servo_units_vel_get(il_servo_t *servo)
{
	il_units_vel_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.vel;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_vel_set(il_servo_t *servo, il_units_vel_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.vel = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_acc_t il_servo_units_acc_get(il_servo_t *servo)
{
	il_units_acc_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.acc;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_acc_set(il_servo_t *servo, il_units_acc_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.acc = units;
	osal_mutex_unlock(servo->units.lock);
}

int il_servo_raw_read(il_servo_t *servo, const il_reg_t *reg, void *buf,
		      size_t sz, size_t *recvd)
{
	assert(servo);
	assert(reg);
	assert(buf);

	if (reg->access == IL_REG_ACCESS_WO) {
		ilerr__set("Register is write-only");
		return IL_EACCESS;
	}

	/* read */
	return il_net__read(servo->net, servo->id, reg->idx, reg->sidx, buf, sz,
			    recvd, servo->timeout);
}

int il_servo_raw_read_u8(il_servo_t *servo, const il_reg_t *reg, uint8_t *buf)
{
	return il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
}

int il_servo_raw_read_s8(il_servo_t *servo, const il_reg_t *reg, int8_t *buf)
{
	return il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
}

int il_servo_raw_read_u16(il_servo_t *servo, const il_reg_t *reg, uint16_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_16(*buf);

	return r;
}

int il_servo_raw_read_s16(il_servo_t *servo, const il_reg_t *reg, int16_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int16_t)__swap_16(*buf);

	return r;
}

int il_servo_raw_read_u32(il_servo_t *servo, const il_reg_t *reg, uint32_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_32(*buf);

	return r;
}

int il_servo_raw_read_s32(il_servo_t *servo, const il_reg_t *reg, int32_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int32_t)__swap_32(*buf);

	return r;
}

int il_servo_raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
			  uint64_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_64(*buf);

	return r;
}

int il_servo_raw_read_s64(il_servo_t *servo, const il_reg_t *reg, int64_t *buf)
{
	int r;

	r = il_servo_raw_read(servo, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int64_t)__swap_64(*buf);

	return r;
}

int il_servo_read(il_servo_t *servo, const il_reg_t *reg, double *buf)
{
	int r;

	uint8_t u8_v;
	uint16_t u16_v;
	uint32_t u32_v;
	uint64_t u64_v;
	int8_t s8_v;
	int16_t s16_v;
	int32_t s32_v;

	int64_t buf_;

	assert(buf);

	/* read */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		r = il_servo_raw_read_u8(servo, reg, &u8_v);
		buf_ = (int64_t)u8_v;
		break;
	case IL_REG_DTYPE_S8:
		r = il_servo_raw_read_s8(servo, reg, &s8_v);
		buf_ = (int64_t)s8_v;
		break;
	case IL_REG_DTYPE_U16:
		r = il_servo_raw_read_u16(servo, reg, &u16_v);
		buf_ = (int64_t)u16_v;
		break;
	case IL_REG_DTYPE_S16:
		r = il_servo_raw_read_s16(servo, reg, &s16_v);
		buf_ = (int64_t)s16_v;
		break;
	case IL_REG_DTYPE_U32:
		r = il_servo_raw_read_u32(servo, reg, &u32_v);
		buf_ = (int64_t)u32_v;
		break;
	case IL_REG_DTYPE_S32:
		r = il_servo_raw_read_s32(servo, reg, &s32_v);
		buf_ = (int64_t)s32_v;
		break;
	case IL_REG_DTYPE_U64:
		r = il_servo_raw_read_u64(servo, reg, &u64_v);
		buf_ = (int64_t)u64_v;
		break;
	case IL_REG_DTYPE_S64:
		r = il_servo_raw_read_s64(servo, reg, &buf_);
		break;
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}

	if (r < 0)
		return r;

	/* store converted value to buffer */
	*buf = buf_ * il_servo_units_factor(servo, reg);

	return 0;
}

int il_servo_raw_write(il_servo_t *servo, const il_reg_t *reg, const void *data,
		       size_t sz, int confirmed)
{
	int confirmed_;

	assert(servo);
	assert(reg);

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	/* skip confirmation on write-only registers */
	confirmed_ = (reg->access == IL_REG_ACCESS_WO) ? 0 : confirmed;

	return il_net__write(servo->net, servo->id, reg->idx, reg->sidx, data,
			     sz, confirmed_, servo->timeout);
}

int il_servo_raw_write_u8(il_servo_t *servo, const il_reg_t *reg, uint8_t val,
			  int confirm)
{
	return il_servo_raw_write(servo, reg, &val, sizeof(val), confirm);
}

int il_servo_raw_write_s8(il_servo_t *servo, const il_reg_t *reg, int8_t val,
			  int confirm)
{
	return il_servo_raw_write(servo, reg, &val, sizeof(val), confirm);
}

int il_servo_raw_write_u16(il_servo_t *servo, const il_reg_t *reg, uint16_t val,
			   int confirm)
{
	uint16_t val_;

	val_ = __swap_16(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_raw_write_s16(il_servo_t *servo, const il_reg_t *reg, int16_t val,
			   int confirm)
{
	int16_t val_;

	val_ = (int16_t)__swap_16(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_raw_write_u32(il_servo_t *servo, const il_reg_t *reg, uint32_t val,
			   int confirm)
{
	uint32_t val_;

	val_ = __swap_32(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_raw_write_s32(il_servo_t *servo, const il_reg_t *reg, int32_t val,
			   int confirm)
{
	int32_t val_;

	val_ = (int32_t)__swap_32(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_raw_write_u64(il_servo_t *servo, const il_reg_t *reg, uint64_t val,
			   int confirm)
{
	uint64_t val_;

	val_ = __swap_64(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_raw_write_s64(il_servo_t *servo, const il_reg_t *reg, int64_t val,
			   int confirm)
{
	int64_t val_;

	val_ = (int64_t)__swap_64(val);

	return il_servo_raw_write(servo, reg, &val_, sizeof(val_), confirm);
}

int il_servo_write(il_servo_t *servo, const il_reg_t *reg, double val,
		   int confirm)
{
	int64_t val_;

	assert(servo);
	assert(reg);

	/* convert to native units */
	val_ = (int64_t)(val / il_servo_units_factor(servo, reg));

	/* write using the appropriate native type */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		return il_servo_raw_write_u8(servo, reg, (uint8_t)val_,
					     confirm);
	case IL_REG_DTYPE_S8:
		return il_servo_raw_write_s8(servo, reg, (int8_t)val_,
					     confirm);
	case IL_REG_DTYPE_U16:
		return il_servo_raw_write_u16(servo, reg, (uint16_t)val_,
					      confirm);
	case IL_REG_DTYPE_S16:
		return il_servo_raw_write_s16(servo, reg, (int16_t)val_,
					      confirm);
	case IL_REG_DTYPE_U32:
		return il_servo_raw_write_u32(servo, reg, (uint32_t)val_,
					      confirm);
	case IL_REG_DTYPE_S32:
		return il_servo_raw_write_s32(servo, reg, (int32_t)val_,
					      confirm);
	case IL_REG_DTYPE_U64:
		return il_servo_raw_write_u64(servo, reg, (uint64_t)val_,
					      confirm);
	case IL_REG_DTYPE_S64:
		return il_servo_raw_write_s64(servo, reg, (int64_t)val_,
					      confirm);
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}
}

il_servo_state_t il_servo_state_get(il_servo_t *servo)
{
	assert(servo);

	return pds_state_decode(sw_get(servo));
}

int il_servo_disable(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;

	assert(servo);

	do {
		sw = sw_get(servo);
		state = pds_state_decode(sw);

		/* check if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			ilerr__set("Servo is in fault state");
			return IL_ESTATE;
		}

		/* check state and command action */
		if (state != IL_SERVO_STATE_DISABLED) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   IL_MC_PDS_CMD_DV, 1);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, sw, PDS_TIMEOUT);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_DISABLED);

	return 0;
}

int il_servo_switch_on(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;

	assert(servo);

	do {
		sw = sw_get(servo);
		state = pds_state_decode(sw);

		/* check if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			ilerr__set("Servo is in fault state");
			return IL_ESTATE;
		}

		/* check state and command action */
		if (state != IL_SERVO_STATE_ON) {
			if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SO;
			else if (state == IL_SERVO_STATE_ENABLED)
				cmd = IL_MC_PDS_CMD_DO;
			else
				cmd = IL_MC_PDS_CMD_DV;

			r = il_servo_raw_write_u16(
					servo, &IL_REG_CTL_WORD, cmd, 1);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, sw, timeout);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_ON);

	return 0;
}

int il_servo_enable(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;

	assert(servo);

	do {
		sw = sw_get(servo);
		state = pds_state_decode(sw);

		/* check if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			ilerr__set("Servo is in fault state");
			return IL_ESTATE;
		}

		/* check state and command action */
		if (state != IL_SERVO_STATE_ENABLED) {
			if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SOEO;
			else
				cmd = IL_MC_PDS_CMD_EO;

			r = il_servo_raw_write_u16(
					servo, &IL_REG_CTL_WORD, cmd, 1);
			if (r < 0)
				return r;
		}

		/* wait for state change */
		if ((state != IL_SERVO_STATE_ENABLED) ||
		    !(sw & IL_MC_SW_IANGLE)) {
			r = sw_wait_change(servo, sw, timeout);
			if (r < 0)
				return r;
		}
	} while ((state != IL_SERVO_STATE_ENABLED) || !(sw & IL_MC_SW_IANGLE));

	return 0;
}

int il_servo_fault_reset(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;

	assert(servo);

	do {
		sw = sw_get(servo);
		state = pds_state_decode(sw);

		/* check if faulty, if so try to reset (0->1) */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   0, 1);
			if (r < 0)
				return r;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   IL_MC_PDS_CMD_FR, 1);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, sw, PDS_TIMEOUT);
			if (r < 0)
				return r;
		}
	} while ((state == IL_SERVO_STATE_FAULT) ||
		 (state == IL_SERVO_STATE_FAULTR));

	return 0;
}

int il_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	return il_servo_raw_write_s8(servo, &IL_REG_OP_MODE, (int8_t)mode, 1);
}

int il_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	return il_servo_read(servo, &IL_REG_OL_VOLTAGE, voltage);
}

int il_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	return il_servo_write(servo, &IL_REG_OL_VOLTAGE, voltage, 1);
}

int il_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	return il_servo_read(servo, &IL_REG_OL_FREQUENCY, freq);
}

int il_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	return il_servo_write(servo, &IL_REG_OL_FREQUENCY, freq, 1);
}

int il_servo_homing_start(il_servo_t *servo)
{
	return il_servo_raw_write_u16(
			servo, &IL_REG_CTL_WORD,
			IL_MC_HOMING_CW_START | IL_MC_PDS_CMD_EO, 1);
}

int il_servo_homing_wait(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, state;

	assert(servo);

	/* wait until finished */
	do {
		sw = sw_get(servo);
		state = sw & IL_MC_HOMING_STA_MSK;

		if (state == IL_MC_HOMING_STA_INPROG) {
			r = sw_wait_change(servo, sw, timeout);
			if (r < 0)
				return r;
		}
	} while (state == IL_MC_HOMING_STA_INPROG);

	if (state == IL_MC_HOMING_STA_SUCCESS)
		return 0;

	/* report failures */
	if (state == IL_MC_HOMING_STA_INT)
		ilerr__set("Homing procedure is interrupted or not started");
	else if (state == IL_MC_HOMING_STA_ATT)
		ilerr__set("Homing is attained, but target is not reached");
	else if (state == IL_MC_HOMING_STA_ERR_VNZ)
		ilerr__set("Homing error occurred, velocity is not zero");
	else if (state == IL_MC_HOMING_STA_ERR_VZ)
		ilerr__set("Homing error occurred, velocity is zero");

	return IL_EFAIL;
}

int il_servo_torque_get(il_servo_t *servo, double *torque)
{
	return il_servo_read(servo, &IL_REG_TORQUE_ACT, torque);
}

int il_servo_torque_set(il_servo_t *servo, double torque)
{
	return il_servo_write(servo, &IL_REG_TORQUE_TGT, torque, 1);
}

int il_servo_position_get(il_servo_t *servo, double *pos)
{
	return il_servo_read(servo, &IL_REG_POS_ACT, pos);
}

int il_servo_position_set(il_servo_t *servo, double pos, int immediate,
			  int relative)
{
	int r;
	uint16_t cmd;

	/* send position */
	r = il_servo_write(servo, &IL_REG_POS_TGT, pos, 1);
	if (r < 0)
		return r;

	/* new set-point (0->1) */
	cmd = IL_MC_PDS_CMD_EO;
	r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, cmd, 1);
	if (r < 0)
		return r;

	cmd |= IL_MC_PP_CW_NEWSP;

	if (immediate)
		cmd |= IL_MC_PP_CW_IMMEDIATE;

	if (relative)
		cmd |= IL_MC_PP_CW_REL;

	r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, cmd, 1);
	if (r < 0)
		return r;

	return 0;
}

int il_servo_position_wait_ack(il_servo_t *servo, int timeout)
{
	int r;

	assert(servo);

	/* wait for SP acknowledge (->1) */
	r = sw_wait_value(servo, IL_MC_PP_SW_SPACK, IL_MC_PP_SW_SPACK,
			  timeout);
	if (r < 0)
		return r;

	/* clear SP */
	r = il_servo_raw_write_u16(
			servo, &IL_REG_CTL_WORD, IL_MC_PDS_CMD_EO, 1);
	if (r < 0)
		return r;

	/* wait SP clear (->0) */
	return sw_wait_value(servo, IL_MC_PP_SW_SPACK, 0, timeout);
}

int il_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb, ppoles, turnbits;
	uint32_t incrs, revs;

	assert(servo);
	assert(res);

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_POS_SENSOR, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_POS_SENSOR_DIGITAL_ENCODER:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(
				servo, &IL_REG_PRES_MOTOR_REVS, &revs);
		if (r < 0)
			return r;

		*res = incrs / revs;
		break;

	case ILK_POS_SENSOR_DIGITAL_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * DIGITAL_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * ANALOG_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_INPUT:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * ANALOG_INPUT_CONSTANT;
		break;

	case ILK_POS_SENSOR_SINCOS:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_MOTOR_REVS,
					  &revs);
		if (r < 0)
			return r;

		*res = (incrs / revs) * SINCOS_CONSTANT;
		break;

	case ILK_POS_SENSOR_PWM:
		*res = PWM_CONSTANT;
		break;

	case ILK_POS_SENSOR_RESOLVER:
		*res = RESOLVER_CONSTANT;
		break;

	case ILK_POS_SENSOR_SSI:
		r = il_servo_raw_read_u8(
				servo, &IL_REG_SSI_STURNBITS, &turnbits);
		if (r < 0)
			return r;

		*res = 2 << turnbits;
		break;

	default:
		*res = 1;
	}

	return 0;
}

int il_servo_velocity_get(il_servo_t *servo, double *vel)
{
	return il_servo_read(servo, &IL_REG_VEL_ACT, vel);
}

int il_servo_velocity_set(il_servo_t *servo, double vel)
{
	return il_servo_write(servo, &IL_REG_VEL_TGT, vel, 1);
}

int il_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb;
	uint32_t incrs, revs;

	assert(servo);
	assert(res);

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_VEL_SENSOR, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_VEL_SENSOR_POS:
		r = il_servo_position_res_get(servo, res);
		break;

	case ILK_VEL_SENSOR_TACHOMETER:
		r = il_servo_raw_read_u32(servo, &IL_REG_VRES_ENC_INCR, &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(
				servo, &IL_REG_VRES_MOTOR_REVS, &revs);
		if (r < 0)
			return r;

		*res = incrs / revs;
		break;

	default:
		*res = 1;
	}

	return 0;
}

int il_servo_wait_reached(il_servo_t *servo, int timeout)
{
	assert(servo);

	/* wait until target reached */
	return sw_wait_value(servo, IL_MC_SW_TR, IL_MC_SW_TR, timeout);
}
