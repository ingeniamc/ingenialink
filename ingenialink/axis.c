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

#include "axis.h"
#include "mc.h"

#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "ingenialink/err.h"
#include "ingenialink/registers.h"
#include "ingenialink/utils.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Statusword update callback
 *
 * @param [in] ctx
 *	Context (axis_t *).
 * @param [in] sw
 *	Statusword value.
 */
static void sw_update(void *ctx, uint16_t sw)
{
	il_axis_t *axis = ctx;

	osal_mutex_lock(axis->sw.lock);

	if (axis->sw.value != sw) {
		axis->sw.value = sw;
		osal_cond_broadcast(axis->sw.changed);
	}

	osal_mutex_unlock(axis->sw.lock);
}

/**
 * Obtain the current statusword value.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 *
 * @return
 *	Statusword value.
 */
static uint16_t sw_get(il_axis_t *axis)
{
	uint16_t sw;

	osal_mutex_lock(axis->sw.lock);
	sw = axis->sw.value;
	osal_mutex_unlock(axis->sw.lock);

	return sw;
}

/**
 * Wait until the statusword changes its value.
 *
 * @note
 *	The timeout is not an absolute timeout, but an interval timeout.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 * @param [in] sw
 *	Current statusword value.
 * @param [in] timeout
 *	Timeout (ms).
 */
static int sw_wait_change(il_axis_t *axis, uint16_t sw, int timeout)
{
	int r = 0;

	osal_mutex_lock(axis->sw.lock);

	if (axis->sw.value == sw) {
		r = osal_cond_wait(axis->sw.changed, axis->sw.lock, timeout);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
		} else if (r < 0) {
			ilerr__set("Statusword wait change failed");
			r = IL_EFAIL;
		}
	}

	osal_mutex_unlock(axis->sw.lock);

	return r;
}

/**
 * Wait until the statusword has the requested value
 *
 * @note
 *	The timeout is not an absolute timeout, but an interval timeout.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 * @param [in] msk
 *	Statusword mask.
 * @param [in] val
 *	Statusword value.
 * @param [in] timeout
 *	Timeout (ms).
 */
static int sw_wait_value(il_axis_t *axis, uint16_t msk, uint16_t val,
			 int timeout)
{
	int r = 0;
	uint16_t result;

	/* wait until the flag changes to the requested state */
	osal_mutex_lock(axis->sw.lock);

	do {
		result = axis->sw.value & msk;
		if (result != val) {
			r = osal_cond_wait(axis->sw.changed, axis->sw.lock,
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

	osal_mutex_unlock(axis->sw.lock);

	return r;
}

/**
 * Update axis configuration.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int update_config(il_axis_t *axis)
{
	int r;

	uint32_t rated_torque, incrs, revs, ppitch;

	/* obtain rated torque (N) */
	r = il_axis_raw_read_u32(axis, &IL_REG_RATED_TORQUE, &rated_torque);
	if (r < 0)
		return r;

	axis->cfg.rated_torque = (double)rated_torque;

	/* FIXME: Assumes digital halls! */
	/* compute position resolution (counts/rev) */
	r = il_axis_raw_read_u32(axis, &IL_REG_PRES_ENC_INCR, &incrs);
	if (r < 0)
		return r;

	r = il_axis_raw_read_u32(axis, &IL_REG_PRES_MOTOR_REVS, &revs);
	if (r < 0)
		return r;

	axis->cfg.pos_res = (double)(incrs / revs);

	/* compute velocity resolution (counts/rev/s) */
	axis->cfg.vel_res = axis->cfg.pos_res;

	/* compute acceleration resolution (counts/rev/s^2) */
	axis->cfg.acc_res = axis->cfg.pos_res;

	/* store magnetic pole pitch */
	r = il_axis_raw_read_u32(axis, &IL_REG_MOTPARAM_PPITCH, &ppitch);
	if (r < 0)
		return r;

	axis->cfg.ppitch = (double)ppitch / 1000000;

	return 0;
}

/**
 * Obtain the units scale factor.
 *
 * @param [in] axis
 *	IngeniaLink axis.
 * @param [in] reg
 *	Register.
 *
 * @return
 *	Scale factor.
 */
static double units_factor(il_axis_t *axis, const il_reg_t *reg)
{
	double factor;

	switch (reg->phy) {
	case IL_REG_PHY_TORQUE:
		switch (axis->units.torque) {
		case IL_UNITS_TORQUE_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_TORQUE_MN:
			factor = axis->cfg.rated_torque / 1000000.;
			break;
		case IL_UNITS_TORQUE_N:
			factor = axis->cfg.rated_torque / 1000.;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_POS:
		switch (axis->units.pos) {
		case IL_UNITS_POS_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_POS_REV:
			factor = 1. / axis->cfg.pos_res;
			break;
		case IL_UNITS_POS_RAD:
			factor = 2. * M_PI / axis->cfg.pos_res;
			break;
		case IL_UNITS_POS_DEG:
			factor = 360. / axis->cfg.pos_res;
			break;
		case IL_UNITS_POS_UM:
			factor = 1000000. * axis->cfg.ppitch /
				 axis->cfg.pos_res;
			break;
		case IL_UNITS_POS_MM:
			factor = 1000. * axis->cfg.ppitch / axis->cfg.pos_res;
			break;
		case IL_UNITS_POS_M:
			factor = 1. * axis->cfg.ppitch / axis->cfg.pos_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_VEL:
		switch (axis->units.vel) {
		case IL_UNITS_VEL_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_VEL_RPS:
			factor = 1. / axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RPM:
			factor = 60. / axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RAD_S:
			factor = 2. * M_PI / axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_DEG_S:
			factor = 360. / axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_UM_S:
			factor = 1000000. * axis->cfg.ppitch /
				 axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_MM_S:
			factor = 1000. * axis->cfg.ppitch / axis->cfg.vel_res;
			break;
		case IL_UNITS_VEL_M_S:
			factor = 1. * axis->cfg.ppitch / axis->cfg.vel_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_ACC:
		switch (axis->units.acc) {
		case IL_UNITS_ACC_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_ACC_REV_S2:
			factor = 1. / axis->cfg.acc_res;
			break;
		case IL_UNITS_ACC_RAD_S2:
			factor = 2. * M_PI / axis->cfg.acc_res;
			break;
		case IL_UNITS_ACC_DEG_S2:
			factor = 360. / axis->cfg.acc_res;
			break;
		case IL_UNITS_ACC_UM_S2:
			factor = 1000000. * axis->cfg.ppitch /
				 axis->cfg.acc_res;
			break;
		case IL_UNITS_ACC_MM_S2:
			factor = 1000. * axis->cfg.ppitch / axis->cfg.acc_res;
			break;
		case IL_UNITS_ACC_M_S2:
			factor = 1. * axis->cfg.ppitch / axis->cfg.acc_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	default:
		factor = 1.;
		break;
	}

	return factor;
}

/**
 * Decode the PDS state.
 *
 * @param [in] sw
 *	Statusword value.
 *
 * @return
 *	PDS state.
 */
static uint16_t pds_state_decode(uint16_t sw)
{
	if ((sw & IL_MC_PDS_STA_NRTSO_MSK) == IL_MC_PDS_STA_NRTSO)
		return IL_MC_PDS_STA_NRTSO;
	else if ((sw & IL_MC_PDS_STA_SOD_MSK) == IL_MC_PDS_STA_SOD)
		return IL_MC_PDS_STA_SOD;
	else if ((sw & IL_MC_PDS_STA_RTSO_MSK) == IL_MC_PDS_STA_RTSO)
		return IL_MC_PDS_STA_RTSO;
	else if ((sw & IL_MC_PDS_STA_SO_MSK) == IL_MC_PDS_STA_SO)
		return IL_MC_PDS_STA_SO;
	else if ((sw & IL_MC_PDS_STA_OE_MSK) == IL_MC_PDS_STA_OE)
		return IL_MC_PDS_STA_OE;
	else if ((sw & IL_MC_PDS_STA_QSA_MSK) == IL_MC_PDS_STA_QSA)
		return IL_MC_PDS_STA_QSA;
	else if ((sw & IL_MC_PDS_STA_FRA_MSK) == IL_MC_PDS_STA_FRA)
		return IL_MC_PDS_STA_FRA;
	else if ((sw & IL_MC_PDS_STA_F_MSK) == IL_MC_PDS_STA_F)
		return IL_MC_PDS_STA_F;

	return IL_MC_PDS_STA_UNKNOWN;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_axis_t *il_axis_create(il_net_t *net, uint8_t id, int timeout)
{
	int r;

	il_axis_t *axis;
	il_units_t default_units;
	uint16_t sw;

	/* validate network */
	if (!net) {
		ilerr__set("Invalid network (NULL)");
		return NULL;
	}

	/* validate node id */
	if ((id < AXISID_MIN) || (id > AXISID_MAX)) {
		ilerr__set("Axis id out of range");
		return NULL;
	}

	/* allocate axis */
	axis = malloc(sizeof(*axis));
	if (!axis) {
		ilerr__set("Axis allocation failed");
		return NULL;
	}

	axis->net = net;
	axis->id = id;
	axis->timeout = timeout;

	default_units.torque = IL_UNITS_TORQUE_NATIVE;
	default_units.pos = IL_UNITS_POS_NATIVE;
	default_units.vel = IL_UNITS_VEL_NATIVE;
	default_units.acc = IL_UNITS_ACC_NATIVE;

	il_axis_units_set(axis, &default_units);

	/* update axis config */
	r = update_config(axis);
	if (r < 0)
		goto cleanup_axis;

	/* configure statusword subcription */
	axis->sw.lock = osal_mutex_create();
	if (!axis->sw.lock) {
		ilerr__set("Statusword subscriber lock allocation failed");
		goto cleanup_axis;
	}

	axis->sw.changed = osal_cond_create();
	if (!axis->sw.changed) {
		ilerr__set("Statusword subscriber condition allocation failed");
		goto cleanup_sw_lock;
	}

	axis->sw.value = 0;

	r = il_net__sw_subscribe(axis->net, axis->id, sw_update, axis);
	if (r < 0)
		goto cleanup_sw_changed;

	/* trigger update (with manual read) */
	(void)il_axis_raw_read_u16(axis, &IL_REG_STS_WORD, &sw);

	return axis;

cleanup_sw_changed:
	osal_cond_destroy(axis->sw.changed);

cleanup_sw_lock:
	osal_mutex_destroy(axis->sw.lock);

cleanup_axis:
	free(axis);

	return NULL;
}

void il_axis_destroy(il_axis_t *axis)
{
	/* validate axis */
	if (!axis)
		return;

	/* de-allocate resources */
	il_net__sw_unsubscribe(axis->net, axis->id);

	osal_cond_destroy(axis->sw.changed);
	osal_mutex_destroy(axis->sw.lock);

	free(axis);
}

int il_axis_units_set(il_axis_t *axis, const il_units_t *units)
{
	/* validate axis, units */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	if (!units) {
		ilerr__set("Invalid units (NULL)");
		return IL_EFAULT;
	}

	memcpy(&axis->units, units, sizeof(axis->units));

	return 0;
}

int il_axis_units_get(il_axis_t *axis, il_units_t *units)
{
	/* validate axis, units */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	if (!units) {
		ilerr__set("Invalid units (NULL)");
		return IL_EFAULT;
	}

	memcpy(units, &axis->units, sizeof(*units));

	return 0;
}

int il_axis_raw_read(il_axis_t *axis, const il_reg_t *reg, void *buf, size_t sz,
		     size_t *recvd)
{
	/* validate axis, register, buffer */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	if (!reg) {
		ilerr__set("Invalid register (NULL)");
		return IL_EFAULT;
	}

	if (reg->access == IL_REG_ACCESS_WO) {
		ilerr__set("Register is write-only");
		return IL_EACCESS;
	}

	if (!buf) {
		ilerr__set("Invalid buffer (NULL)");
		return IL_EFAULT;
	}

	/* read */
	return il_net__read(axis->net, axis->id, reg->idx, reg->sidx, buf, sz,
			    recvd, axis->timeout);
}

int il_axis_raw_read_u8(il_axis_t *axis, const il_reg_t *reg, uint8_t *buf)
{
	return il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
}

int il_axis_raw_read_s8(il_axis_t *axis, const il_reg_t *reg, int8_t *buf)
{
	return il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
}

int il_axis_raw_read_u16(il_axis_t *axis, const il_reg_t *reg, uint16_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_16(*buf);

	return r;
}

int il_axis_raw_read_s16(il_axis_t *axis, const il_reg_t *reg, int16_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int16_t)__swap_16(*buf);

	return r;
}

int il_axis_raw_read_u32(il_axis_t *axis, const il_reg_t *reg, uint32_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_32(*buf);

	return r;
}

int il_axis_raw_read_s32(il_axis_t *axis, const il_reg_t *reg, int32_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int32_t)__swap_32(*buf);

	return r;
}

int il_axis_raw_read_u64(il_axis_t *axis, const il_reg_t *reg,
			 uint64_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_64(*buf);

	return r;
}

int il_axis_raw_read_s64(il_axis_t *axis, const il_reg_t *reg, int64_t *buf)
{
	int r;

	r = il_axis_raw_read(axis, reg, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int64_t)__swap_64(*buf);

	return r;
}

int il_axis_read(il_axis_t *axis, const il_reg_t *reg, double *buf)
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

	/* validate buffer */
	if (!buf) {
		ilerr__set("Invalid buffer (NULL)");
		return IL_EFAULT;
	}

	/* read */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		r = il_axis_raw_read_u8(axis, reg, &u8_v);
		buf_ = (int64_t)u8_v;
		break;
	case IL_REG_DTYPE_S8:
		r = il_axis_raw_read_s8(axis, reg, &s8_v);
		buf_ = (int64_t)s8_v;
		break;
	case IL_REG_DTYPE_U16:
		r = il_axis_raw_read_u16(axis, reg, &u16_v);
		buf_ = (int64_t)u16_v;
		break;
	case IL_REG_DTYPE_S16:
		r = il_axis_raw_read_s16(axis, reg, &s16_v);
		buf_ = (int64_t)s16_v;
		break;
	case IL_REG_DTYPE_U32:
		r = il_axis_raw_read_u32(axis, reg, &u32_v);
		buf_ = (int64_t)u32_v;
		break;
	case IL_REG_DTYPE_S32:
		r = il_axis_raw_read_s32(axis, reg, &s32_v);
		buf_ = (int64_t)s32_v;
		break;
	case IL_REG_DTYPE_U64:
		r = il_axis_raw_read_u64(axis, reg, &u64_v);
		buf_ = (int64_t)u64_v;
		break;
	case IL_REG_DTYPE_S64:
		r = il_axis_raw_read_s64(axis, reg, &buf_);
		break;
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}

	if (r < 0)
		return r;

	/* store converted value to buffer */
	*buf = buf_ * units_factor(axis, reg);

	return 0;
}

int il_axis_raw_write(il_axis_t *axis, const il_reg_t *reg, const void *data,
		      size_t sz)
{
	/* validate axis, register */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	if (!reg) {
		ilerr__set("Invalid register (NULL)");
		return IL_EFAULT;
	}

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	return il_net__write(axis->net, axis->id, reg->idx, reg->sidx, data,
			     sz);
}

int il_axis_raw_write_u8(il_axis_t *axis, const il_reg_t *reg, uint8_t val)
{
	return il_axis_raw_write(axis, reg, &val, sizeof(val));
}

int il_axis_raw_write_s8(il_axis_t *axis, const il_reg_t *reg, int8_t val)
{
	return il_axis_raw_write(axis, reg, &val, sizeof(val));
}

int il_axis_raw_write_u16(il_axis_t *axis, const il_reg_t *reg, uint16_t val)
{
	uint16_t val_;

	val_ = __swap_16(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_raw_write_s16(il_axis_t *axis, const il_reg_t *reg, int16_t val)
{
	int16_t val_;

	val_ = (int16_t)__swap_16(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_raw_write_u32(il_axis_t *axis, const il_reg_t *reg, uint32_t val)
{
	uint32_t val_;

	val_ = __swap_32(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_raw_write_s32(il_axis_t *axis, const il_reg_t *reg, int32_t val)
{
	int32_t val_;

	val_ = (int32_t)__swap_32(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_raw_write_u64(il_axis_t *axis, const il_reg_t *reg, uint64_t val)
{
	uint64_t val_;

	val_ = __swap_64(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_raw_write_s64(il_axis_t *axis, const il_reg_t *reg, int64_t val)
{
	int64_t val_;

	val_ = (int64_t)__swap_64(val);

	return il_axis_raw_write(axis, reg, &val_, sizeof(val_));
}

int il_axis_write(il_axis_t *axis, const il_reg_t *reg, double val)
{
	int64_t val_;

	/* validate arguments */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	if (!reg) {
		ilerr__set("Invalid register (NULL)");
		return IL_EFAULT;
	}

	/* convert to native units */
	val_ = (int64_t)(val / units_factor(axis, reg));

	/* write using the appropriate native type */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		return il_axis_raw_write_u8(axis, reg, (uint8_t)val_);
	case IL_REG_DTYPE_S8:
		return il_axis_raw_write_s8(axis, reg, (int8_t)val_);
	case IL_REG_DTYPE_U16:
		return il_axis_raw_write_u16(axis, reg, (uint16_t)val_);
	case IL_REG_DTYPE_S16:
		return il_axis_raw_write_s16(axis, reg, (int16_t)val_);
	case IL_REG_DTYPE_U32:
		return il_axis_raw_write_u32(axis, reg, (uint32_t)val_);
	case IL_REG_DTYPE_S32:
		return il_axis_raw_write_s32(axis, reg, (int32_t)val_);
	case IL_REG_DTYPE_U64:
		return il_axis_raw_write_u64(axis, reg, (uint64_t)val_);
	case IL_REG_DTYPE_S64:
		return il_axis_raw_write_s64(axis, reg, (int64_t)val_);
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}
}

int il_axis_disable(il_axis_t *axis)
{
	int r;
	uint16_t sw, state;

	do {
		sw = sw_get(axis);
		state = pds_state_decode(sw);

		/* check if faulty or unknown */
		if ((state == IL_MC_PDS_STA_F) ||
		    (state == IL_MC_PDS_STA_FRA)) {
			ilerr__set("Axis is in fault state");
			return IL_EFAIL;
		}

		if (state == IL_MC_PDS_STA_UNKNOWN) {
			ilerr__set("Axis state is unknown");
			return IL_ESTATE;
		}

		/* check state and command action */
		if (state != IL_MC_PDS_STA_SOD) {
			r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD,
						  IL_MC_PDS_CMD_DV);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(axis, sw, PDS_TIMEOUT);
			if (r < 0)
				return r;
		}
	} while (state != IL_MC_PDS_STA_SOD);

	return 0;
}

int il_axis_enable(il_axis_t *axis)
{
	int r;
	uint16_t sw, state, cmd;

	do {
		sw = sw_get(axis);
		state = pds_state_decode(sw);

		/* check if faulty or unknown */
		if ((state == IL_MC_PDS_STA_F) ||
		    (state == IL_MC_PDS_STA_FRA)) {
			ilerr__set("Axis is in fault state");
			return IL_ESTATE;
		}

		if (state == IL_MC_PDS_STA_UNKNOWN) {
			ilerr__set("Axis state is unknown");
			return IL_ESTATE;
		}

		/* check state and command action */
		if (state != IL_MC_PDS_STA_OE) {
			if (state == IL_MC_PDS_STA_NRTSO)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_MC_PDS_STA_SOD)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_MC_PDS_STA_RTSO)
				cmd = IL_MC_PDS_CMD_SOEO;
			else
				cmd = IL_MC_PDS_CMD_EO;

			r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD, cmd);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(axis, sw, PDS_TIMEOUT);
			if (r < 0)
				return r;
		}
	} while (state != IL_MC_PDS_STA_OE);

	return 0;
}

int il_axis_fault_reset(il_axis_t *axis)
{
	int r;
	uint16_t sw, state;

	do {
		sw = sw_get(axis);
		state = pds_state_decode(sw);

		/* check if faulty, if so try to reset */
		if ((state == IL_MC_PDS_STA_F) ||
		    (state == IL_MC_PDS_STA_FRA)) {
			r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD,
						  IL_MC_PDS_CMD_FR);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(axis, sw, PDS_TIMEOUT);
			if (r < 0)
				return r;
		}
	} while ((state == IL_MC_PDS_STA_F) || (state == IL_MC_PDS_STA_FRA));

	return 0;
}

int il_axis_mode_set(il_axis_t *axis, il_axis_mode_t mode)
{
	return il_axis_raw_write_u8(axis, &IL_REG_OP_MODE, (uint8_t)mode);
}

int il_axis_homing_start(il_axis_t *axis)
{
	return il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD,
				     IL_MC_HOMING_CW_START | IL_MC_PDS_CMD_EO);
}

int il_axis_homing_wait(il_axis_t *axis, int timeout)
{
	int r;
	uint16_t sw, state;

	/* validate axis */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	/* wait until finished */
	do {
		sw = sw_get(axis);
		state = sw & IL_MC_HOMING_STA_MSK;

		if (state == IL_MC_HOMING_STA_INPROG) {
			r = sw_wait_change(axis, sw, timeout);
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

int il_axis_torque_get(il_axis_t *axis, double *torque)
{
	return il_axis_read(axis, &IL_REG_TORQUE_ACT, torque);
}

int il_axis_torque_set(il_axis_t *axis, double torque)
{
	return il_axis_write(axis, &IL_REG_TORQUE_TGT, torque);
}

int il_axis_position_get(il_axis_t *axis, double *pos)
{
	return il_axis_read(axis, &IL_REG_POS_ACT, pos);
}

int il_axis_position_set(il_axis_t *axis, double pos, int immediate,
			 int relative)
{
	int r;
	uint16_t cmd;

	/* send position */
	r = il_axis_write(axis, &IL_REG_POS_TGT, pos);
	if (r < 0)
		return r;

	/* new set-point (0->1) */
	cmd = IL_MC_PDS_CMD_EO;
	r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD, cmd);
	if (r < 0)
		return r;

	cmd |= IL_MC_PP_CW_NEWSP;

	if (immediate)
		cmd |= IL_MC_PP_CW_IMMEDIATE;

	if (relative)
		cmd |= IL_MC_PP_CW_REL;

	r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD, cmd);
	if (r < 0)
		return r;

	/* wait for set-point acknowledge (1) */
	r = sw_wait_value(axis, IL_MC_PP_SW_SPACK, IL_MC_PP_SW_SPACK,
			  SPACK_TIMEOUT);
	if (r < 0)
		return r;

	/* clear set point (1->0) */
	cmd = IL_MC_PDS_CMD_EO;
	r = il_axis_raw_write_u16(axis, &IL_REG_CTL_WORD, cmd);
	if (r < 0)
		return r;

	/* wait for set-point acknowledge clear (0) */
	return sw_wait_value(axis, IL_MC_PP_SW_SPACK, 0, SPACK_TIMEOUT);
}

int il_axis_velocity_get(il_axis_t *axis, double *vel)
{
	return il_axis_read(axis, &IL_REG_VEL_ACT, vel);
}

int il_axis_velocity_set(il_axis_t *axis, double vel)
{
	return il_axis_write(axis, &IL_REG_VEL_TGT, vel);
}

int il_axis_wait_reached(il_axis_t *axis, int timeout)
{
	/* validate axis */
	if (!axis) {
		ilerr__set("Invalid axis (NULL)");
		return IL_EFAULT;
	}

	/* wait until target reached */
	return sw_wait_value(axis, IL_MC_SW_TR, IL_MC_SW_TR, timeout);
}
