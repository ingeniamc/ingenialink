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
#include "mc.h"

#include <ctype.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "public/ingenialink/const.h"
#include "ingenialink/err.h"
#include "ingenialink/registers.h"
#include "ingenialink/base/servo.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

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
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in, out] sw
 *	Current statusword value, where next value will be stored.
 * @param [in, out] timeout
 *	Timeout (ms), if positive will be updated with remaining ms.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int sw_wait_change(il_servo_t *servo, uint16_t *sw, int *timeout)
{
	int r = 0;
	osal_timespec_t start = { 0, 0 }, end, diff;

	/* obtain start time */
	if (*timeout > 0) {
		if (osal_clock_gettime(&start) < 0) {
			ilerr__set("Could not obtain system time");
			return IL_EFAIL;
		}
	}

	/* wait for change */
	osal_mutex_lock(servo->sw.lock);

	if (servo->sw.value == *sw) {
		r = osal_cond_wait(servo->sw.changed, servo->sw.lock, *timeout);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
			goto out;
		} else if (r < 0) {
			ilerr__set("Statusword wait change failed");
			r = IL_EFAIL;
			goto out;
		}
	}

	*sw = servo->sw.value;

out:
	/* update timeout */
	if ((*timeout > 0) && (r == 0)) {
		/* obtain end time */
		if (osal_clock_gettime(&end) < 0) {
			ilerr__set("Could not obtain system time");
			r = IL_EFAIL;
			goto unlock;
		}

		/* compute difference */
		if ((end.ns - start.ns) < 0) {
			diff.s = end.s - start.s - 1;
			diff.ns = end.ns - start.ns + OSAL_CLOCK_NANOSPERSEC;
		} else {
			diff.s = end.s - start.s;
			diff.ns = end.ns - start.ns;
		}

		/* update timeout */
		*timeout -= diff.s * 1000 + diff.ns / OSAL_CLOCK_NANOSPERMSEC;
		if (*timeout <= 0) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
		}
	}

unlock:
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
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_eusb_servo_t *).
 */
static void servo_destroy(void *ctx)
{
	il_eusb_servo_t *this = ctx;

	il_servo_base__deinit(&this->servo);

	free(this);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_eusb_servo__retain(il_servo_t *servo)
{
	il_eusb_servo_t *this = to_eusb_servo(servo);

	il_utils__refcnt_retain(this->refcnt);
}

void il_eusb_servo__release(il_servo_t *servo)
{
	il_eusb_servo_t *this = to_eusb_servo(servo);

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
void il_eusb_servo__state_decode(uint16_t sw, il_servo_state_t *state,
				 int *flags)
{
	if ((sw & IL_MC_PDS_STA_NRTSO_MSK) == IL_MC_PDS_STA_NRTSO)
		*state = IL_SERVO_STATE_NRDY;
	else if ((sw & IL_MC_PDS_STA_SOD_MSK) == IL_MC_PDS_STA_SOD)
		*state = IL_SERVO_STATE_DISABLED;
	else if ((sw & IL_MC_PDS_STA_RTSO_MSK) == IL_MC_PDS_STA_RTSO)
		*state = IL_SERVO_STATE_RDY;
	else if ((sw & IL_MC_PDS_STA_SO_MSK) == IL_MC_PDS_STA_SO)
		*state = IL_SERVO_STATE_ON;
	else if ((sw & IL_MC_PDS_STA_OE_MSK) == IL_MC_PDS_STA_OE)
		*state = IL_SERVO_STATE_ENABLED;
	else if ((sw & IL_MC_PDS_STA_QSA_MSK) == IL_MC_PDS_STA_QSA)
		*state = IL_SERVO_STATE_QSTOP;
	else if ((sw & IL_MC_PDS_STA_FRA_MSK) == IL_MC_PDS_STA_FRA)
		*state = IL_SERVO_STATE_FAULTR;
	else if ((sw & IL_MC_PDS_STA_F_MSK) == IL_MC_PDS_STA_F)
		*state = IL_SERVO_STATE_FAULT;
	else
		*state = IL_SERVO_STATE_NRDY;

	if (flags)
		*flags = (int)(sw >> FLAGS_SW_POS);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

static il_servo_t *il_eusb_servo_create(il_net_t *net, uint16_t id,
					const char *dict)
{
	int r;

	il_eusb_servo_t *this;
	uint16_t sw;

	/* validate node id */
	if ((id < SERVOID_MIN) || (id > SERVOID_MAX)) {
		ilerr__set("Servo id out of range");
		return NULL;
	}

	/* allocate servo */
	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}

	r = il_servo_base__init(&this->servo, net, id, dict);
	if (r < 0)
		goto cleanup_servo;

	this->servo.ops = &il_eusb_servo_ops;

	/* initialize, setup refcnt */
	this->refcnt = il_utils__refcnt_create(servo_destroy, this);
	if (!this->refcnt)
		goto cleanup_base;

	/* obtain current operation mode */
	r = il_servo_mode_get(&this->servo, &this->servo.mode);
	if (r < 0)
		goto cleanup_refcnt;

	r = il_servo_units_update(&this->servo);
	if (r < 0)
		goto cleanup_refcnt;

	/* trigger status update (with manual read) */
	(void)il_servo_raw_read_u16(&this->servo, &IL_REG_STS_WORD, NULL, &sw);

	return &this->servo;

cleanup_refcnt:
	il_utils__refcnt_destroy(this->refcnt);

cleanup_base:
	il_servo_base__deinit(&this->servo);

cleanup_servo:
	free(this);

	return NULL;
}

static void il_eusb_servo_destroy(il_servo_t *servo)
{
	il_eusb_servo_t *this = to_eusb_servo(servo);

	il_utils__refcnt_release(this->refcnt);
}

static int il_eusb_servo_reset(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_RESET_DEVICE, NULL,
				      ILK_SIGNATURE_RESET, 0, 0);
}

static int il_eusb_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	int r;
	uint64_t name_;

	if (sz < IL_SERVO_NAME_SZ) {
		ilerr__set("Insufficient name buffer size");
		return IL_ENOMEM;
	}

	/* QUIRK: some firmware do not have the name feature */
	r = il_servo_raw_read_u64(servo, &IL_REG_DRIVE_NAME, NULL, &name_);
	if (r == 0 && name_) {
		memcpy(name, &name_, sizeof(name_));
		name[IL_SERVO_NAME_SZ - 1] = '\0';
	} else {
		strncpy(name, "Default", sz);
	}

	return 0;
}

static int il_eusb_servo_name_set(il_servo_t *servo, const char *name)
{
	size_t sz;
	uint64_t name_ = 0;

	/* clip name to the maximum size */
	sz = MIN(strlen(name), sizeof(name_));
	memcpy(&name_, name, sz);

	return il_servo_raw_write_u64(
		servo, &IL_REG_DRIVE_NAME, NULL, name_, 1, 0);
}

static int il_eusb_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
{
	int r;
	size_t i;

	r = il_servo_raw_read_u32(servo, &IL_REG_ID_SERIAL, NULL,
				  &info->serial);
	if (r < 0)
		return r;

	r = il_servo_name_get(servo, info->name, sizeof(info->name));
	if (r < 0)
		return r;

	memset(info->sw_version, 0, sizeof(info->sw_version));
	r = il_servo_raw_read_u64(servo, &IL_REG_SW_VERSION, NULL,
				  (uint64_t *)info->sw_version);
	if (r < 0)
		return r;

	memset(info->hw_variant, 0, sizeof(info->hw_variant));
	r = il_servo_raw_read_u32(servo, &IL_REG_HW_VARIANT, NULL,
				  (uint32_t *)info->hw_variant);
	/* QUIRK: some firmwares do not have this register */
	if (r == 0) {
		/* QUIRK: hardware variant may not be present in all devices. If
		 * not present it may contain random non-printable characters,
		 * so make it null.
		 */
		for (i = 0; i < sizeof(info->hw_variant); i++) {
			if (info->hw_variant[i] != '\0' &&
			    !isprint((int)info->hw_variant[i])) {
				memset(info->hw_variant, 0,
				       sizeof(info->hw_variant));
				break;
			}
		}
	}

	r = il_servo_raw_read_u32(servo, &IL_REG_ID_PROD_CODE, NULL,
				  &info->prod_code);
	if (r < 0)
		return r;

	return il_servo_raw_read_u32(servo, &IL_REG_ID_REVISION, NULL,
				     &info->revision);
}

static int il_eusb_servo_store_all(il_servo_t *servo, int subnode)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_ALL, NULL,
				      ILK_SIGNATURE_STORE, 0, 0);
}

static int il_eusb_servo_store_comm(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_COMM, NULL,
				      ILK_SIGNATURE_STORE, 0, 0);
}

static int il_eusb_servo_store_app(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_APP, NULL,
				      ILK_SIGNATURE_STORE, 0, 0);
}

static int il_eusb_servo_units_update(il_servo_t *servo)
{
	int r;
	uint16_t motor_type;
	uint32_t rated_torque, pos_res, vel_res, dist_scale;

	r = il_servo_raw_read_u32(servo, &IL_REG_RATED_TORQUE, NULL,
				  &rated_torque);
	if (r < 0)
		return r;

	r = il_servo_position_res_get(servo, &pos_res);
	if (r < 0)
		return r;

	r = il_servo_velocity_res_get(servo, &vel_res);
	if (r < 0)
		return r;

	/* distance constant (pole pitch for rotary, stroke for linear) */
	r = il_servo_raw_read_u16(servo, &IL_REG_MOTOR_TYPE, NULL, &motor_type);
	if (r < 0)
		return r;

	switch (motor_type) {
	/* linear */
	case ILK_MOTOR_LIN_BLAC:
	case ILK_MOTOR_LIN_BLDC:
	case ILK_MOTOR_LIN_VC:
	case ILK_MOTOR_LIN_DC:
		r = il_servo_raw_read_u32(servo, &IL_REG_MOTPARAM_STROKE, NULL,
					  &dist_scale);
		if (r < 0)
			return r;

		break;
	/* rotary (or default) */
	default:
		r = il_servo_raw_read_u32(servo, &IL_REG_MOTPARAM_PPITCH, NULL,
					  &dist_scale);
		if (r < 0)
			return r;
	}

	servo->cfg.rated_torque = (double)rated_torque;
	servo->cfg.pos_res = (double)pos_res;
	servo->cfg.vel_res = (double)vel_res;
	servo->cfg.acc_res = servo->cfg.pos_res;
	servo->cfg.dist_scale = (double)dist_scale / 1000000;

	return 0;
}

static double il_eusb_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	double factor;

	osal_mutex_lock(servo->units.lock);

	switch (reg->phy) {
	case IL_REG_PHY_TORQUE:
		switch (servo->units.torque) {
		case IL_UNITS_TORQUE_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_TORQUE_MNM:
			factor = servo->cfg.rated_torque / 1000000.;
			break;
		case IL_UNITS_TORQUE_NM:
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
			factor = 1000000. * servo->cfg.dist_scale /
				 servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_MM:
			factor = 1000. * servo->cfg.dist_scale /
				 servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_M:
			factor = 1. * servo->cfg.dist_scale /
				 servo->cfg.pos_res;
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
			factor = 1000000. * servo->cfg.dist_scale /
				 servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_MM_S:
			factor = 1000. * servo->cfg.dist_scale /
				 servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_M_S:
			factor = 1. * servo->cfg.dist_scale /
				 servo->cfg.vel_res;
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
			factor = 1000000. * servo->cfg.dist_scale /
				 servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_MM_S2:
			factor = 1000. * servo->cfg.dist_scale /
				 servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_M_S2:
			factor = 1. * servo->cfg.dist_scale /
				 servo->cfg.acc_res;
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

static int il_eusb_servo_disable(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;

	sw = sw_get(servo);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_eusb_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach disabled */
		} else if (state != IL_SERVO_STATE_DISABLED) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_DV, 1, 0);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_DISABLED);

	return 0;
}

static int il_eusb_servo_switch_on(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	sw = sw_get(servo);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_eusb_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach switch on */
		} else if (state != IL_SERVO_STATE_ON) {
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

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, cmd, 1, 0);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_ON);

	return 0;
}

static int il_eusb_servo_enable(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	sw = sw_get(servo);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_eusb_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach enabled */
		} else if ((state != IL_SERVO_STATE_ENABLED) ||
			   !(sw & IL_MC_SW_IANGLE)) {
			if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SOEO;
			else
				cmd = IL_MC_PDS_CMD_EO;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, cmd, 1, 0);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_);
			if (r < 0)
				return r;
		}
	} while ((state != IL_SERVO_STATE_ENABLED) || !(sw & IL_MC_SW_IANGLE));

	return 0;
}

static int il_eusb_servo_fault_reset(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;

	sw = sw_get(servo);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* check if faulty, if so try to reset (0->1) */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, 0, 1, 0);
			if (r < 0)
				return r;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_FR, 1, 0);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout);
			if (r < 0)
				return r;
		}
	} while ((state == IL_SERVO_STATE_FAULT) ||
		 (state == IL_SERVO_STATE_FAULTR));

	return 0;
}

static int il_eusb_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	int r;
	int8_t code;

	r = il_servo_raw_read_s8(servo, &IL_REG_OP_MODE_DISP, NULL, &code);
	if (r < 0)
		return r;

	switch (code) {
	case ILK_OP_MODE_OLV:
		*mode = IL_SERVO_MODE_OLV;
		break;
	case ILK_OP_MODE_OLS:
		*mode = IL_SERVO_MODE_OLS;
		break;
	case ILK_OP_MODE_PP:
		*mode = IL_SERVO_MODE_PP;
		break;
	case ILK_OP_MODE_VEL:
		*mode = IL_SERVO_MODE_VEL;
		break;
	case ILK_OP_MODE_PV:
		*mode = IL_SERVO_MODE_PV;
		break;
	case ILK_OP_MODE_PT:
		*mode = IL_SERVO_MODE_PT;
		break;
	case ILK_OP_MODE_HOMING:
		*mode = IL_SERVO_MODE_HOMING;
		break;
	case ILK_OP_MODE_IP:
		*mode = IL_SERVO_MODE_IP;
		break;
	case ILK_OP_MODE_CSP:
		*mode = IL_SERVO_MODE_CSP;
		break;
	case ILK_OP_MODE_CSV:
		*mode = IL_SERVO_MODE_CSV;
		break;
	case ILK_OP_MODE_CST:
		*mode = IL_SERVO_MODE_CST;
		break;
	default:
		/* assume PP if unknown */
		*mode = IL_SERVO_MODE_PP;
		break;
	}

	return 0;
}

static int il_eusb_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	int r;
	int8_t code;

	switch (mode) {
	case IL_SERVO_MODE_OLV:
		code = ILK_OP_MODE_OLV;
		break;
	case IL_SERVO_MODE_OLS:
		code = ILK_OP_MODE_OLS;
		break;
	case IL_SERVO_MODE_PP:
		code = ILK_OP_MODE_PP;
		break;
	case IL_SERVO_MODE_VEL:
		code = ILK_OP_MODE_VEL;
		break;
	case IL_SERVO_MODE_PV:
		code = ILK_OP_MODE_PV;
		break;
	case IL_SERVO_MODE_PT:
		code = ILK_OP_MODE_PT;
		break;
	case IL_SERVO_MODE_HOMING:
		code = ILK_OP_MODE_HOMING;
		break;
	case IL_SERVO_MODE_IP:
		code = ILK_OP_MODE_IP;
		break;
	case IL_SERVO_MODE_CSP:
		code = ILK_OP_MODE_CSP;
		break;
	case IL_SERVO_MODE_CSV:
		code = ILK_OP_MODE_CSV;
		break;
	case IL_SERVO_MODE_CST:
		code = ILK_OP_MODE_CST;
		break;
	default:
		ilerr__set("Invalid mode");
		return IL_EINVAL;
	}

	r = il_servo_raw_write_s8(servo, &IL_REG_OP_MODE, NULL, code, 1, 0);
	if (r < 0)
		return r;

	servo->mode = mode;

	return 0;
}

static int il_eusb_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	return il_servo_read(servo, &IL_REG_OL_VOLTAGE, NULL, voltage);
}

static int il_eusb_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	return il_servo_write(servo, &IL_REG_OL_VOLTAGE, NULL, voltage, 1, 0);
}

static int il_eusb_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	return il_servo_read(servo, &IL_REG_OL_FREQUENCY, NULL, freq);
}

static int il_eusb_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	return il_servo_write(servo, &IL_REG_OL_FREQUENCY, NULL, freq, 1, 0);
}

static int il_eusb_servo_homing_start(il_servo_t *servo)
{
	return il_servo_raw_write_u16(
			servo, &IL_REG_CTL_WORD, NULL,
			IL_MC_HOMING_CW_START | IL_MC_PDS_CMD_EO, 1, 0);
}

static int il_eusb_servo_homing_wait(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, state;
	int timeout_ = timeout;

	sw = sw_get(servo);

	do {
		state = sw & IL_MC_HOMING_STA_MSK;

		if (state == IL_MC_HOMING_STA_INPROG) {
			r = sw_wait_change(servo, &sw, &timeout_);
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

static int il_eusb_servo_torque_get(il_servo_t *servo, double *torque)
{
	return il_servo_read(servo, &IL_REG_TORQUE_ACT, NULL, torque);
}

static int il_eusb_servo_torque_set(il_servo_t *servo, double torque)
{
	return il_servo_write(servo, &IL_REG_TORQUE_TGT, NULL, torque, 1, 0);
}

static int il_eusb_servo_position_get(il_servo_t *servo, double *pos)
{
	return il_servo_read(servo, &IL_REG_POS_ACT, NULL, pos);
}

static int il_eusb_servo_position_set(il_servo_t *servo, double pos,
				      int immediate, int relative,
				      int sp_timeout)
{
	int r;
	uint16_t cmd;
	il_servo_state_t state;
	int flags;

	/* send position */
	r = il_servo_write(servo, &IL_REG_POS_TGT, NULL, pos, 1, 0);
	if (r < 0)
		return r;

	/* wait for SP ack if enabled and in PP */
	il_servo_state_get(servo, &state, &flags, 1);

	if ((state == IL_SERVO_STATE_ENABLED) &&
	    (servo->mode == IL_SERVO_MODE_PP)) {
		/* new set-point (0->1) */
		cmd = IL_MC_PDS_CMD_EO;
		r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, NULL,
					   cmd, 1, 0);
		if (r < 0)
			return r;

		/* wait set-point ack clear */
		r = sw_wait_value(servo, IL_MC_PP_SW_SPACK, 0, sp_timeout);
		if (r < 0)
			return r;

		/* set-point */
		cmd |= IL_MC_PP_CW_NEWSP;

		if (immediate)
			cmd |= IL_MC_PP_CW_IMMEDIATE;

		if (relative)
			cmd |= IL_MC_PP_CW_REL;

		r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, NULL,
					   cmd, 1, 0);
		if (r < 0)
			return r;

		/* wait set-point ack */
		r = sw_wait_value(servo, IL_MC_PP_SW_SPACK, IL_MC_PP_SW_SPACK,
				  sp_timeout);
		if (r < 0)
			return r;
	}

	return 0;
}

static int il_eusb_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb, ppoles, turnbits;
	uint32_t incrs, revs;

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_POS_SENSOR, NULL, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_POS_SENSOR_DIGITAL_ENCODER:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		/* avoid zero division on invalid values */
		if (revs == 0)
			*res = 1;
		else
			*res = incrs / revs;

		break;

	case ILK_POS_SENSOR_DIGITAL_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, NULL,
					 &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * DIGITAL_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, NULL,
					 &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * ANALOG_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_INPUT:
		*res = ANALOG_INPUT_CONSTANT;
		break;

	case ILK_POS_SENSOR_SINCOS:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		/* avoid zero division on invalid values */
		if (revs == 0)
			*res = 1;
		else
			*res = (incrs / revs) * SINCOS_CONSTANT;

		break;

	case ILK_POS_SENSOR_PWM:
		*res = PWM_CONSTANT;
		break;

	case ILK_POS_SENSOR_RESOLVER:
		*res = RESOLVER_CONSTANT;
		break;

	case ILK_POS_SENSOR_SSI:
		r = il_servo_raw_read_u8(servo, &IL_REG_SSI_STURNBITS, NULL,
					 &turnbits);
		if (r < 0)
			return r;

		*res = 2 << turnbits;
		break;

	default:
		*res = 1;
	}

	/* fix resolution to non-zero (to avoid divisions by zero) in case some
	 * of the registers contain invalid values.
	 */
	if (*res == 0)
		*res = 1;

	return 0;
}

static int il_eusb_servo_velocity_get(il_servo_t *servo, double *vel)
{
	return il_servo_read(servo, &IL_REG_VEL_ACT, NULL, vel);
}

static int il_eusb_servo_velocity_set(il_servo_t *servo, double vel)
{
	return il_servo_write(servo, &IL_REG_VEL_TGT, NULL, vel, 1, 0);
}

static int il_eusb_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb;
	uint32_t incrs, revs;

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_VEL_SENSOR, NULL, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_VEL_SENSOR_POS:
		r = il_servo_position_res_get(servo, res);
		break;

	case ILK_VEL_SENSOR_TACHOMETER:
		r = il_servo_raw_read_u32(servo, &IL_REG_VRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_VRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		/* avoid zero division on invalid values */
		if (revs == 0)
			*res = 1;
		else
			*res = incrs / revs;

		break;

	default:
		*res = 1;
	}

	/* fix resolution to non-zero (to avoid divisions by zero) in case some
	 * of the registers contain invalid values.
	 */
	if (*res == 0)
		*res = 1;

	return 0;
}

static int il_eusb_servo_wait_reached(il_servo_t *servo, int timeout)
{
	return sw_wait_value(servo, IL_MC_SW_TR, IL_MC_SW_TR, timeout);
}

/** E-USB servo operations. */
const il_servo_ops_t il_eusb_servo_ops = {
	/* internal */
	._retain = il_eusb_servo__retain,
	._release = il_eusb_servo__release,
	._state_decode = il_eusb_servo__state_decode,
	/* public */
	.create = il_eusb_servo_create,
	.destroy = il_eusb_servo_destroy,
	.reset = il_eusb_servo_reset,
	.state_get = il_servo_base__state_get,
	.state_subscribe = il_servo_base__state_subscribe,
	.state_unsubscribe = il_servo_base__state_unsubscribe,
	.emcy_subscribe = il_servo_base__emcy_subscribe,
	.emcy_unsubscribe = il_servo_base__emcy_unsubscribe,
	.dict_get = il_servo_base__dict_get,
	.dict_load = il_servo_base__dict_load,
	.name_get = il_eusb_servo_name_get,
	.name_set = il_eusb_servo_name_set,
	.info_get = il_eusb_servo_info_get,
	.store_all = il_eusb_servo_store_all,
	.store_comm = il_eusb_servo_store_comm,
	.store_app = il_eusb_servo_store_app,
	.units_update = il_eusb_servo_units_update,
	.units_factor = il_eusb_servo_units_factor,
	.units_torque_get = il_servo_base__units_torque_get,
	.units_torque_set = il_servo_base__units_torque_set,
	.units_pos_get = il_servo_base__units_pos_get,
	.units_pos_set = il_servo_base__units_pos_set,
	.units_vel_get = il_servo_base__units_vel_get,
	.units_vel_set = il_servo_base__units_vel_set,
	.units_acc_get = il_servo_base__units_acc_get,
	.units_acc_set = il_servo_base__units_acc_set,
	.raw_read_u8 = il_servo_base__raw_read_u8,
	.raw_read_s8 = il_servo_base__raw_read_s8,
	.raw_read_u16 = il_servo_base__raw_read_u16,
	.raw_read_s16 = il_servo_base__raw_read_s16,
	.raw_read_u32 = il_servo_base__raw_read_u32,
	.raw_read_s32 = il_servo_base__raw_read_s32,
	.raw_read_u64 = il_servo_base__raw_read_u64,
	.raw_read_s64 = il_servo_base__raw_read_s64,
	.raw_read_float = il_servo_base__raw_read_float,
	.read = il_servo_base__read,
	.raw_write_u8 = il_servo_base__raw_write_u8,
	.raw_write_s8 = il_servo_base__raw_write_s8,
	.raw_write_u16 = il_servo_base__raw_write_u16,
	.raw_write_s16 = il_servo_base__raw_write_s16,
	.raw_write_u32 = il_servo_base__raw_write_u32,
	.raw_write_s32 = il_servo_base__raw_write_s32,
	.raw_write_u64 = il_servo_base__raw_write_u64,
	.raw_write_s64 = il_servo_base__raw_write_s64,
	.raw_write_float = il_servo_base__raw_write_float,
	.write = il_servo_base__write,
	.disable = il_eusb_servo_disable,
	.switch_on = il_eusb_servo_switch_on,
	.enable = il_eusb_servo_enable,
	.fault_reset = il_eusb_servo_fault_reset,
	.mode_get = il_eusb_servo_mode_get,
	.mode_set = il_eusb_servo_mode_set,
	.ol_voltage_get = il_eusb_servo_ol_voltage_get,
	.ol_voltage_set = il_eusb_servo_ol_voltage_set,
	.ol_frequency_get = il_eusb_servo_ol_frequency_get,
	.ol_frequency_set = il_eusb_servo_ol_frequency_set,
	.homing_start = il_eusb_servo_homing_start,
	.homing_wait = il_eusb_servo_homing_wait,
	.torque_get = il_eusb_servo_torque_get,
	.torque_set = il_eusb_servo_torque_set,
	.position_get = il_eusb_servo_position_get,
	.position_set = il_eusb_servo_position_set,
	.position_res_get = il_eusb_servo_position_res_get,
	.velocity_get = il_eusb_servo_velocity_get,
	.velocity_set = il_eusb_servo_velocity_set,
	.velocity_res_get = il_eusb_servo_velocity_res_get,
	.wait_reached = il_eusb_servo_wait_reached,
};
