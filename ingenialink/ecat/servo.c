
#include "servo.h"
#include "mc.h"

#include <string.h>

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "ingenialink/err.h"
#include "ingenialink/base/servo.h"
#include "ingenialink/registers.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
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
static uint16_t sw_get(il_servo_t *servo, uint8_t subnode)
{
	// Init internal variables
	double sw;
	// Status word register
	il_reg_t status_word_register = {
		.subnode = subnode,
		.address = 0x0011,
		.dtype = IL_REG_DTYPE_U16,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_NONE,
		.range = {
			.min.u16 = 0,
			.max.u16 = UINT16_MAX
		},
		.labels = NULL,
		.enums = NULL,
		.enums_count = 0
	};
	(void)il_servo_read(servo, &status_word_register, NULL, &sw);
	if (servo->sw.value != sw) {
		servo->sw.value = sw;
		osal_cond_broadcast(servo->sw.changed);
	}

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
static int sw_wait_change(il_servo_t *servo, uint16_t *sw, int *timeout, uint8_t subnode)
{
	int r = 0;
	uint16_t buff;
	osal_timespec_t start = { 0, 0 }, end, diff;
	// Status word register
	il_reg_t status_word_register = {
		.subnode = subnode,
		.address = 0x0011,
		.dtype = IL_REG_DTYPE_U16,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_NONE,
		.range = {
			.min.u16 = 0,
			.max.u16 = UINT16_MAX
		},
		.labels = NULL,
		.enums = NULL,
		.enums_count = 0
	};

	/* obtain start time */
	if (*timeout > 0) {
		if (osal_clock_gettime(&start) < 0) {
			ilerr__set("Could not obtain system time");
			return IL_EFAIL;
		}
	}
	double time_s = 0;
	time_s = (double) *timeout / 1000;
	(void)il_servo_raw_read_u16(servo, &status_word_register, NULL, &buff);
	while (buff == *sw) {
		osal_clock_gettime(&diff);
		if (diff.s > start.s + time_s) {
			ilerr__set("Operation timed out");
 			r = IL_ETIMEDOUT;
 			goto unlock;
		}
		(void)il_servo_raw_read_u16(servo, &status_word_register, NULL, &buff);
	}

	servo->sw.value = buff;
	*sw = buff;

out:	

unlock:

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


	return r;
}


/**
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_ecat_servo_t *).
 */
static void servo_destroy(void *ctx)
{
	printf("Servo destroyed!\n");
	il_ecat_servo_t *this = ctx;

	il_servo_base__deinit(&this->servo);

	free(this);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_ecat_servo__retain(il_servo_t *servo)
{
	il_ecat_servo_t *this = to_ecat_servo(servo);

	il_utils__refcnt_retain(this->refcnt);
}

void il_ecat_servo__release(il_servo_t *servo)
{
	il_ecat_servo_t *this = to_ecat_servo(servo);

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
void il_ecat_servo__state_decode(uint16_t sw, il_servo_state_t *state,
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

static il_servo_t *il_ecat_servo_create(il_net_t *net, uint16_t id,
				       const char *dict)
{
	int r;
	il_ecat_servo_t *this;
	double sw;
	
	/* allocate servo */
	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}
	r = il_servo_base__init(&this->servo, net, id, dict);
	if (r < 0) {
		goto cleanup_servo;
	}

	this->servo.state_subs.stop = 1;
    (void)osal_thread_join(this->servo.state_subs.monitor, NULL);
		
	this->servo.ops = &il_ecat_servo_ops;

	/* Configure the number of axis if the register is defined */
	uint16_t subnodes;
	r = il_servo_raw_read_u16(&this->servo, &IL_REG_ETH_NUMBER_AXIS, NULL, &subnodes);
	if (r < 0) {
		this->servo.subnodes = 1;
	}
	else {
		this->servo.subnodes = subnodes;
	}

	/* initialize, setup refcnt */
	this->refcnt = il_utils__refcnt_create(servo_destroy, this);

	if (!this->refcnt)
		goto cleanup_base;

	/* trigger status update (with manual read) */
	(void)il_servo_read(&this->servo, &IL_REG_MCB_STS_WORD, NULL, &sw);

	return &this->servo;

cleanup_base:
	il_servo_base__deinit(&this->servo);

cleanup_servo:
	free(this);

	return NULL;
}

static void il_ecat_servo_destroy(il_servo_t *servo)
{
	il_ecat_servo_t *this = to_ecat_servo(servo);

	il_utils__refcnt_release(this->refcnt);
}

static int il_ecat_servo_reset(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_ecat_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	(void)servo;

	strncpy(name, "Drive", sz);

	return 0;
}

static int il_ecat_servo_name_set(il_servo_t *servo, const char *name)
{
	(void)servo;
	(void)name;

	return not_supported();
}

static int il_ecat_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
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

static int il_ecat_servo_store_comm(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_ecat_servo_store_app(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_ecat_servo_units_update(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static double il_ecat_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	(void)servo;
	(void)reg;

	return not_supported();
}

static int il_ecat_servo_disable(il_servo_t *servo, uint8_t subnode)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;
	sw = sw_get(servo, subnode);
	
	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_fault_reset(servo, subnode);
			if (r < 0)
				return r;

			sw = sw_get(servo, subnode);
		/* check state and command action to reach disabled */
		} else if (state != IL_SERVO_STATE_DISABLED) {
			IL_REG_MCB_CTL_WORD.subnode = subnode;
			r = il_servo_raw_write_u16(servo, &IL_REG_MCB_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_DV, 1, 0);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout, subnode);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_DISABLED);

	return 0;
}

static int il_ecat_servo_switch_on(il_servo_t *servo, int timeout, uint8_t subnode)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	sw = sw_get(servo, subnode);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_fault_reset(servo, subnode);
			if (r < 0)
				return r;

			sw = sw_get(servo, subnode);
		/* check state and command action to reach switch on */
		} else if (state != IL_SERVO_STATE_ON) {
			if (state == IL_SERVO_STATE_FAULT)
				return IL_ESTATE;
			else if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SO;
			else if (state == IL_SERVO_STATE_ENABLED)
				cmd = IL_MC_PDS_CMD_DO;
			else
				cmd = IL_MC_PDS_CMD_DV;

			IL_REG_MCB_CTL_WORD.subnode = subnode;
			r = il_servo_raw_write_u16(servo, &IL_REG_MCB_CTL_WORD,
						   NULL, cmd, 1, 0);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_, subnode);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_ON);

	return 0;
}

static int il_ecat_servo_enable(il_servo_t *servo, int timeout, uint8_t subnode)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	sw = sw_get(servo, subnode);

	servo->ops->_state_decode(sw, &state, NULL);

	/* try fault reset if faulty */
	if ((state == IL_SERVO_STATE_FAULT) ||
		(state == IL_SERVO_STATE_FAULTR)) {
		r = il_servo_fault_reset(servo, subnode);
		if (r < 0)
			return r;

		sw = sw_get(servo, subnode);
	}

	sw = sw_get(servo, subnode);
	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* check state and command action to reach enabled */
		if ((state != IL_SERVO_STATE_ENABLED)) {
			if (state == IL_SERVO_STATE_FAULT)
				return IL_ESTATE;
			else if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SOEO;
			else
				cmd = IL_MC_PDS_CMD_EO;

			IL_REG_MCB_CTL_WORD.subnode = subnode;
			r = il_servo_raw_write_u16(servo, &IL_REG_MCB_CTL_WORD,
						   NULL, cmd, 1, 0);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_, subnode);
			if (r < 0)
				return r;
	
		}
	} while ((state != IL_SERVO_STATE_ENABLED));

	return 0;
}

static int il_ecat_servo_fault_reset(il_servo_t *servo, uint8_t subnode)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;
	int retries = 0;

	sw = sw_get(servo, subnode);

	do {
		servo->ops->_state_decode(sw, &state, NULL);

		/* check if faulty, if so try to reset (0->1) */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			if (retries == FAULT_RESET_RETRIES) {
				return IL_ESTATE;
			}
			
			IL_REG_MCB_CTL_WORD.subnode = subnode;
			r = il_servo_raw_write_u16(servo, &IL_REG_MCB_CTL_WORD,
						   NULL, 0, 1, 0);
			if (r < 0)
				return r;

			IL_REG_MCB_CTL_WORD.subnode = subnode;
			r = il_servo_raw_write_u16(servo, &IL_REG_MCB_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_FR, 1, 0);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout, subnode);
			if (r < 0)
				return r;

			++retries;
		}
	} while ((state == IL_SERVO_STATE_FAULT) ||
		 (state == IL_SERVO_STATE_FAULTR));

	return 0;
}

static int il_ecat_servo_store_all(il_servo_t *servo, int subnode)
{
	int r = 0;

	// Set subnode in store all register
	il_reg_t il_reg_store_all = IL_REG_ETH_STORE_ALL;
	il_reg_store_all.subnode = subnode;

	r = il_servo_raw_wait_write_u32(servo, &il_reg_store_all,
						   NULL, 0x65766173, 1, 0);

	printf("Store finished!");

	return r;
}

static int il_ecat_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	(void)servo;
	(void)mode;

	return not_supported();
}

static int il_ecat_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	(void)servo;
	(void)mode;

	return not_supported();
}

static int il_ecat_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_ecat_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	(void)servo;
	(void)voltage;

	return not_supported();
}

static int il_ecat_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_ecat_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	(void)servo;
	(void)freq;

	return not_supported();
}

static int il_ecat_servo_homing_start(il_servo_t *servo)
{
	(void)servo;

	return not_supported();
}

static int il_ecat_servo_homing_wait(il_servo_t *servo, int timeout)
{
	(void)servo;
	(void)timeout;

	return not_supported();
}

static int il_ecat_servo_torque_get(il_servo_t *servo, double *torque)
{
	(void)servo;
	(void)torque;

	return not_supported();
}

static int il_ecat_servo_torque_set(il_servo_t *servo, double torque)
{
	(void)servo;
	(void)torque;

	return not_supported();
}

static int il_ecat_servo_position_get(il_servo_t *servo, double *pos)
{
	(void)servo;
	(void)pos;

	return not_supported();
}

static int il_ecat_servo_position_set(il_servo_t *servo, double pos,
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

static int il_ecat_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_ecat_servo_velocity_get(il_servo_t *servo, double *vel)
{
	(void)servo;
	(void)vel;

	return not_supported();
}

static int il_ecat_servo_velocity_set(il_servo_t *servo, double vel)
{
	(void)servo;
	(void)vel;

	return not_supported();
}

static int il_ecat_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	(void)servo;
	(void)res;

	return not_supported();
}

static int il_ecat_servo_wait_reached(il_servo_t *servo, int timeout)
{
	return sw_wait_value(servo, IL_MC_SW_TR, IL_MC_SW_TR, timeout);
}


/** ECAT servo operations. */
const il_servo_ops_t il_ecat_servo_ops = {
	/* internal */
	._retain = il_ecat_servo__retain,
	._release = il_ecat_servo__release,
	._state_decode = il_ecat_servo__state_decode,
    /* public */
	.create = il_ecat_servo_create,
	.destroy = il_ecat_servo_destroy,
	.reset = il_ecat_servo_reset,
	.state_get = il_servo_base__state_get,
	.state_subscribe = il_servo_base__state_subscribe,
	.state_unsubscribe = il_servo_base__state_unsubscribe,
	.emcy_subscribe = il_servo_base__emcy_subscribe,
	.emcy_unsubscribe = il_servo_base__emcy_unsubscribe,
	.dict_get = il_servo_base__dict_get,
	.dict_load = il_servo_base__dict_load,
	.name_get = il_ecat_servo_name_get,
	.name_set = il_ecat_servo_name_set,
	.info_get = il_ecat_servo_info_get,
	.store_comm = il_ecat_servo_store_comm,
	.store_app = il_ecat_servo_store_app,
	.units_update = il_ecat_servo_units_update,
	.units_factor = il_ecat_servo_units_factor,
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
	.raw_read_str = il_servo_base__raw_read_str,
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
	.raw_wait_write_u32 = il_servo_base__raw_wait_write_u32,
	.raw_write_s32 = il_servo_base__raw_write_s32,
	.raw_write_u64 = il_servo_base__raw_write_u64,
	.raw_write_s64 = il_servo_base__raw_write_s64,
	.raw_write_float = il_servo_base__raw_write_float,
	.write = il_servo_base__write,
	.disable = il_ecat_servo_disable,
	.switch_on = il_ecat_servo_switch_on,
	.enable = il_ecat_servo_enable,
	.fault_reset = il_ecat_servo_fault_reset,
	.store_all = il_ecat_servo_store_all,
	.mode_get = il_ecat_servo_mode_get,
	.mode_set = il_ecat_servo_mode_set,
	.ol_voltage_get = il_ecat_servo_ol_voltage_get,
	.ol_voltage_set = il_ecat_servo_ol_voltage_set,
	.ol_frequency_get = il_ecat_servo_ol_frequency_get,
	.ol_frequency_set = il_ecat_servo_ol_frequency_set,
	.homing_start = il_ecat_servo_homing_start,
	.homing_wait = il_ecat_servo_homing_wait,
	.torque_get = il_ecat_servo_torque_get,
	.torque_set = il_ecat_servo_torque_set,
	.position_get = il_ecat_servo_position_get,
	.position_set = il_ecat_servo_position_set,
	.position_res_get = il_ecat_servo_position_res_get,
	.velocity_get = il_ecat_servo_velocity_get,
	.velocity_set = il_ecat_servo_velocity_set,
	.velocity_res_get = il_ecat_servo_velocity_res_get,
	.wait_reached = il_ecat_servo_wait_reached,
};