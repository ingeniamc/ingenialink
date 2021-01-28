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

#ifndef PUBLIC_INGENIALINK_SERVO_H_
#define PUBLIC_INGENIALINK_SERVO_H_

#include "net.h"
#include "dict.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/servo.h
 * @brief Servo.
 * @defgroup IL_SERVO Servo
 * @ingroup IL
 * @{
 */

/** IngeniaLink servo instance. */
typedef struct il_servo il_servo_t;

/** Set-point acknowledge default timeout (ms). */
#define IL_SERVO_SP_TIMEOUT_DEF	1000

/** Servo name size (includes null termination). */
#define IL_SERVO_NAME_SZ	9

/** Servo software version size. */
#define IL_SERVO_SW_VERSION_SZ	9

/** Hardware variant size. */
#define IL_SERVO_HW_VARIANT_SZ	5

/** IngeniaLink servo information. */
typedef struct {
	/** Serial. */
	uint32_t serial;
	/** Name. */
	char name[IL_SERVO_NAME_SZ];
	/** Software version. */
	char sw_version[IL_SERVO_SW_VERSION_SZ];
	/** Hardware variant. */
	char hw_variant[IL_SERVO_HW_VARIANT_SZ];
	/** Product code. */
	uint32_t prod_code;
	/** Revision number. */
	uint32_t revision;
} il_servo_info_t;

/** Emergency subscriber callback. */
typedef void (*il_servo_emcy_subscriber_cb_t)(void *ctx, uint32_t code);

/** Servo states (equivalent to CiA 402 PDS states). */
typedef enum {
	/** Not ready to switch on. */
	IL_SERVO_STATE_NRDY,
	/** Switch on disabled. */
	IL_SERVO_STATE_DISABLED,
	/** Ready to be switched on. */
	IL_SERVO_STATE_RDY,
	/** Power switched on. */
	IL_SERVO_STATE_ON,
	/** Enabled. */
	IL_SERVO_STATE_ENABLED,
	/** Quick stop. */
	IL_SERVO_STATE_QSTOP,
	/** Fault reactive. */
	IL_SERVO_STATE_FAULTR,
	/** Fault. */
	IL_SERVO_STATE_FAULT
} il_servo_state_t;

/*
 * Servo state flags.
 *
 * NOTES:
 *	Depending on the operation mode, the flags bits 3-4 have different
 *	meanings (as in CiA402). The following definitions can be used to test
 *	the flags.
 */

/** Flags: Target reached. */
#define IL_SERVO_FLAG_TGT_REACHED	0x01
/** Flags: Internal limit active. */
#define IL_SERVO_FLAG_ILIM_ACTIVE	0x02
/** Flags: (Homing): attained. */
#define IL_SERVO_FLAG_HOMING_ATT	0x04
/** Flags: (Homing): error. */
#define IL_SERVO_FLAG_HOMING_ERR	0x08
/** Flags: (PV): Vocity speed is zero. */
#define IL_SERVO_FLAG_PV_VZERO		0x04
/** Flags: (PP): SP acknowledge. */
#define IL_SERVO_FLAG_PP_SPACK		0x04
/** Flags: (IP): active. */
#define IL_SERVO_FLAG_IP_ACTIVE		0x04
/** Flags: (CST/CSV/CSP): follow command value. */
#define IL_SERVO_FLAG_CS_FOLLOWS	0x04
/** Flags: (CST/CSV/CSP/PV): following error. */
#define IL_SERVO_FLAG_FERR		0x08
/** Flags: Initial angle determination finished. */
#define IL_SERVO_FLAG_IANGLE_DET	0x10

/** State updates subcriber callback. */
typedef void (*il_servo_state_subscriber_cb_t)(
		void *ctx, il_servo_state_t state, int flags, uint8_t subnode);

/** Servo operation modes. */
typedef enum {
	/** Open loop (vector mode). */
	IL_SERVO_MODE_OLV,
	/** Open loop (scalar mode). */
	IL_SERVO_MODE_OLS,
	/** Profile position. */
	IL_SERVO_MODE_PP,
	/** Velocity */
	IL_SERVO_MODE_VEL,
	/** Profile velocity. */
	IL_SERVO_MODE_PV,
	/** Profile torque. */
	IL_SERVO_MODE_PT,
	/** Homing. */
	IL_SERVO_MODE_HOMING,
	/** Interpolated position. */
	IL_SERVO_MODE_IP,
	/** Cyclic sync position mode. */
	IL_SERVO_MODE_CSP,
	/** Cyclic sync velocity mode. */
	IL_SERVO_MODE_CSV,
	/** Cyclic sync torque mode. */
	IL_SERVO_MODE_CST,
} il_servo_mode_t;

/** Torque units. */
typedef enum {
	/** Native. */
	IL_UNITS_TORQUE_NATIVE,
	/** Millinewtons*m. */
	IL_UNITS_TORQUE_MNM,
	/** Newtons*m. */
	IL_UNITS_TORQUE_NM
} il_units_torque_t;

/** Position units. */
typedef enum {
	/** Native. */
	IL_UNITS_POS_NATIVE,
	/** Revolutions. */
	IL_UNITS_POS_REV,
	/** Radians. */
	IL_UNITS_POS_RAD,
	/** Degrees. */
	IL_UNITS_POS_DEG,
	/** Micrometers. */
	IL_UNITS_POS_UM,
	/** Millimeters. */
	IL_UNITS_POS_MM,
	/** Meters. */
	IL_UNITS_POS_M,
} il_units_pos_t;

/** Velocity units. */
typedef enum {
	/** Native. */
	IL_UNITS_VEL_NATIVE,
	/** Revolutions per second. */
	IL_UNITS_VEL_RPS,
	/** Revolutions per minute. */
	IL_UNITS_VEL_RPM,
	/** Radians/second. */
	IL_UNITS_VEL_RAD_S,
	/** Degrees/second. */
	IL_UNITS_VEL_DEG_S,
	/** Micrometers/second. */
	IL_UNITS_VEL_UM_S,
	/** Millimeters/second. */
	IL_UNITS_VEL_MM_S,
	/** Meters/second. */
	IL_UNITS_VEL_M_S,
} il_units_vel_t;

/** Acceleration units. */
typedef enum {
	/** Native. */
	IL_UNITS_ACC_NATIVE,
	/** Revolutions/second^2. */
	IL_UNITS_ACC_REV_S2,
	/** Radians/second^2. */
	IL_UNITS_ACC_RAD_S2,
	/** Degrees/second^2. */
	IL_UNITS_ACC_DEG_S2,
	/** Micrometers/second^2. */
	IL_UNITS_ACC_UM_S2,
	/** Millimeters/second^2. */
	IL_UNITS_ACC_MM_S2,
	/** Meters/second^2. */
	IL_UNITS_ACC_M_S2,
} il_units_acc_t;

/**
 * Create IngeniaLink servo instance.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Servo id.
 * @param [in] dict
 *	Dictionary (optional).
 *
 * @return
 *	Servo instance (NULL if it could not be created).
 */
IL_EXPORT il_servo_t *il_servo_create(il_net_t *net, uint16_t id, const char *dict);

/**
 * Destroy an IngeniaLink servo instance.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 */
IL_EXPORT void il_servo_destroy(il_servo_t *servo);

/**
 * Reset servo.
 *
 * @notes
 *	You may need to reconnect the network after a reset.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_reset(il_servo_t *servo);

/**
 * Obtain current servo PDS state.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] state
 *	Servo state.
 * @param [out] flags
 *	Servo flags.
 * @param [in] subnode
 *	Subnode.
 */

IL_EXPORT void il_servo_state_get(il_servo_t *servo, il_servo_state_t *state,
				  int *flags, uint8_t subnode);

/**
 * Subscribe to state changes (and operation flags).
 *
 * @note
 *	Callbacks should be relatively fast, otherwise consecutive state changes
 *	may be lost (as there is no buffering). Note that callbacks are
 *	decoupled from the communications thread so that they do not affect
 *	other operations.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [in] cb
 *	State and operation flags change callback.
 * @param [in] ctx
 *	Callback context (optional).
 *
 * @return
 *	Assigned slot (>= 0) or error code (< 0).
 */
IL_EXPORT int il_servo_state_subscribe(il_servo_t *servo,
				       il_servo_state_subscriber_cb_t cb,
				       void *ctx);

/**
 * Unsubscribe from state changes.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [in] slot
 *	Assigned subscription slot.
 */
IL_EXPORT void il_servo_state_unsubscribe(il_servo_t *servo, int slot);

/**
 * Subscribe to emergency messages.
 *
 * @note
 *	Callbacks should be relatively fast, otherwise some emergencies
 *	may be lost. Although an internal queue is used, it is limited. Note
 *	that callbacks are decoupled from the communications thread so that they
 *	do not affect other operations.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] cb
 *	Callback.
 * @param [in] ctx
 *	Callback context.
 *
 * @returns
 *	Assigned slot (>= 0) or error code (< 0).
 */
IL_EXPORT int il_servo_emcy_subscribe(il_servo_t *servo,
				      il_servo_emcy_subscriber_cb_t cb,
				      void *ctx);

/**
 * Unubscribe from emergency messages.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] slot
 *	Assigned subscription slot.
 */
IL_EXPORT void il_servo_emcy_unsubscribe(il_servo_t *servo, int slot);

/**
 * Obtain servo dictionary.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 *
 * @return
 *	Servo dictionary (NULL if none is loaded).
 */
IL_EXPORT il_dict_t *il_servo_dict_get(il_servo_t *servo);

/**
 * Load a dictionary to the servo.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [in] dict
 *	Dictionary.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_dict_load(il_servo_t *servo, const char *dict);

/**
 * Read all dictionary registers content and put it to the dictionary storage.
 *
 * @param [in] servo
 *	Servo instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_dict_storage_read(il_servo_t *servo);

/**
 * Write current dictionary storage to the servo drive.
 *
 * @param [in] servo
 *	Servo instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_dict_storage_write(il_servo_t *servo);

/**
 * Obtain servo name.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [out] name
 *	Buffer where the actual name (null-terminated) will be stored.
 * @param [in] sz
 *	Buffer size (must be >= IL_SERVO_NAME_SZ).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_name_get(il_servo_t *servo, char *name, size_t sz);

/**
 * Set servo name.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [in] name
 *	Servo name (null-terminated string).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_name_set(il_servo_t *servo, const char *name);

/**
 * Obtain servo information.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [out] info
 *	Buffer where servo information will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_info_get(il_servo_t *servo, il_servo_info_t *info);

/**
 * Store all servo current parameters to the NVM.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_store_all(il_servo_t *servo, int subnode);

/**
 * Store all servo current communications parameters to the NVM.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 * @param [in] subnode
 *	Subnode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_store_comm(il_servo_t *servo);

/**
 * Store all servo current application parameters to the NVM.
 *
 * @param [in] servo
 *	IngeniaLink servo instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_store_app(il_servo_t *servo);

/**
 * Update units scaling factors.
 *
 * @note
 *	This must be called if any encoder parameter, rated torque or pole pitch
 *	are changed, otherwise, the readings conversions will not be correct.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_units_update(il_servo_t *servo);

/**
 * Obtain the units scale factor associated with the given register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 *
 * @return
 *	Scale factor.
 */
IL_EXPORT double il_servo_units_factor(il_servo_t *servo, const il_reg_t *reg);

/**
 * Get the torque units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Torque units (IL_UNITS_TORQUE_NATIVE if servo is not valid).
 */
IL_EXPORT il_units_torque_t il_servo_units_torque_get(il_servo_t *servo);

/**
 * Set the torque units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] units
 *	Units.
 */
IL_EXPORT void il_servo_units_torque_set(il_servo_t *servo,
					 il_units_torque_t units);

/**
 * Get the position units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Position units (IL_UNITS_POS_NATIVE if servo is not valid).
 */
IL_EXPORT il_units_pos_t il_servo_units_pos_get(il_servo_t *servo);

/**
 * Set the position units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] units
 *	Units.
 */
IL_EXPORT void il_servo_units_pos_set(il_servo_t *servo, il_units_pos_t units);

/**
 * Get the velocity units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Velocity units (IL_UNITS_VEL_NATIVE if servo is not valid).
 */
IL_EXPORT il_units_vel_t il_servo_units_vel_get(il_servo_t *servo);

/**
 * Set the velocity units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] units
 *	Units.
 */
IL_EXPORT void il_servo_units_vel_set(il_servo_t *servo, il_units_vel_t units);

/**
 * Get the acceleration units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Acceleration units (IL_UNITS_ACC_NATIVE if servo is not valid).
 */
IL_EXPORT il_units_acc_t il_servo_units_acc_get(il_servo_t *servo);

/**
 * Set the acceleration units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] units
 *	Units.
 */
IL_EXPORT void il_servo_units_acc_set(il_servo_t *servo, il_units_acc_t units);

/**
 * Read unsigned 8-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_u8(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, uint8_t *buf);

/**
 * Read signed 8-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_s8(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, int8_t *buf);

/**
 * Read unsigned 16-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, uint16_t *buf);

/**
 * Read signed 16-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, int16_t *buf);

/**
 * Read unsigned 32-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, uint32_t *buf);

/**
 * Read string value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_str(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, uint32_t *buf);

/**
 * Read signed 32-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, int32_t *buf);

/**
 * Read unsigned 64-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, uint64_t *buf);

/**
 * Read signed 64-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, int64_t *buf);

/**
 * Read signed 64-bit value from a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where to store received data.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_read_float(il_servo_t *servo, const il_reg_t *reg,
				      const char *id, float *buf);

/**
 * Read a register.
 *
 * This function will read from a register and also perform a unit conversion to
 * match the current operating units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register id.
 * @param [out] buf
 *	Buffer where adjusted register content will be stored.
 *
 * @returns
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_read(il_servo_t *servo, const il_reg_t *reg,
			    const char *id, double *buf);

/**
 * Write unsigned 8-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, uint8_t val, int confirm, uint16_t extended);

/**
 * Write signed 8-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
				    const char *id, int8_t val, int confirm, uint16_t extended);

/**
 * Write unsigned 16-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, uint16_t val, int confirm, uint16_t extended);

/**
 * Write signed 16-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, int16_t val, int confirm, uint16_t extended);

/**
 * Write unsigned 32-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, uint32_t val, int confirm, uint16_t extended);

IL_EXPORT int il_servo_raw_wait_write_u32(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, uint32_t val, int confirm, uint16_t extended);

/**
 * Write signed 32-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, int32_t val, int confirm, uint16_t extended);

/**
 * Write unsigned 64-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, uint64_t val, int confirm, uint16_t extended);

/**
 * Write signed 64-bit integer to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
				     const char *id, int64_t val, int confirm, uint16_t extended);

/**
 * Write float to a register.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_raw_write_float(il_servo_t *servo, const il_reg_t *reg,
				       const char *id, float val, int confirm, uint16_t extended);

/**
 * Write to a register.
 *
 * This function will write to a register and also perform a unit conversion to
 * match the current operating units.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] reg
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] val
 *	Value.
 * @param [in] confirm
 *	Confirm the write.
 *
 * @returns
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_write(il_servo_t *servo, const il_reg_t *reg,
			     const char *id, double val, int confirm, uint16_t extended);

/**
 * Disable servo PDS.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_disable(il_servo_t *servo, uint8_t subnode);

/**
 * Switch on servo PDS.
 *
 * @note
 *	The timeout is the timeout between state changes.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] subnode
 *	Subnode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_switch_on(il_servo_t *servo, int timeout, uint8_t subnode);

/**
 * Enable servo PDS.
 *
 * @note
 *	The timeout is the timeout between state changes.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] timeout
 *	Timeout (ms).
 * @param [in] subnode
 *	Subnode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_enable(il_servo_t *servo, int timeout, uint8_t subnode);

/**
 * Reset the drive fault state.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] timeout
 *	Timeout (ms).
 * @param [in] subnode
 *	Subnode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_fault_reset(il_servo_t *servo, uint8_t subnode);

/**
 * Get the servo operation mode.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] mode
 *	Where mode will be stored.
 * @param [in] subnode
 *	Subnode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode);

/**
 * Set the servo operation mode.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] mode
 *	Mode.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode);

/**
 * Get the open loop voltage.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] voltage
 *	Voltage buffer (% relative to DC-Bus, -1...1).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_ol_voltage_get(il_servo_t *servo, double *voltage);

/**
 * Set the open loop voltage.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] voltage
 *	Voltage (% relative to DC-Bus, -1...1).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_ol_voltage_set(il_servo_t *servo, double voltage);

/**
 * Get the open loop frequency.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] freq
 *	Frequency buffer (mHz).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_ol_frequency_get(il_servo_t *servo, double *freq);

/**
 * Set the open loop frequency.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] freq
 *	Frequency (mHz).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_ol_frequency_set(il_servo_t *servo, double freq);

/**
 * Start homing.
 *
 * @note
 *	This assumes that the drive is in the appropriate mote and that the PDS
 *	is enabled.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_homing_start(il_servo_t *servo);

/**
 * Wait until homing completes.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_homing_wait(il_servo_t *servo, int timeout);

/**
 * Get the actual servo torque.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] torque
 *	Where the actual torque will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_torque_get(il_servo_t *servo, double *torque);

/**
 * Set the servo target torque.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] torque
 *	Torque.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_torque_set(il_servo_t *servo, double torque);

/**
 * Get the actual servo position.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] pos
 *	Where the actual position will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_position_get(il_servo_t *servo, double *pos);

/**
 * Set the servo target position.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] pos
 *	Position.
 * @param [in] immediate
 *	If set, the position will be set immediately, otherwise will be added
 *	to the internal servo queue.
 * @param [in] relative
 *	If set, the position is taken as a relative value, otherwise it is taken
 *	as an absolute value.
 * @param [in] sp_timeout
 *	Set-point acknowledge timeout (ms).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_position_set(il_servo_t *servo, double pos,
				    int immediate, int relative,
				    int sp_timeout);

/**
 * Obtain position resolution.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] res
 *	Position resolution (c/rev, c/ppitch).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_position_res_get(il_servo_t *servo, uint32_t *res);

/**
 * Get the actual servo velocity.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] vel
 *	Where the actual velocity will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_velocity_get(il_servo_t *servo, double *vel);

/**
 * Set the servo target velocity.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] vel
 *	Velocity.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_velocity_set(il_servo_t *servo, double vel);

/**
 * Obtain velocity resolution.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [out] res
 *	Velocity resolution (c/rev/s).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_velocity_res_get(il_servo_t *servo, uint32_t *res);

/**
 * Wait until the servo does a target reach.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] timeout
 *	Timeout (ms).
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_servo_wait_reached(il_servo_t *servo, int timeout);


IL_EXPORT void il_servo_fake_destroy(il_servo_t *servo);

/**
 * Utility function to connect to the first available servo drive.
 *
 * @param [in] prot
 *	Network protocol.
 * @param [out] net
 *	Where the servo network will be stored.
 * @param [out] servo
 *	Where the first available servo will be stored.
 * @param [in] dict
 *	Dictionary (optional).
 *
 * @return
 *	0 if a servo is found, IL_EFAIL if none are found.
 */
IL_EXPORT int il_servo_lucky(il_net_prot_t prot, il_net_t **net,
			     il_servo_t **servo, const char *dict);

IL_EXPORT int il_servo_lucky_eth(il_net_prot_t prot, il_net_t **net,
			     il_servo_t **servo, const char *dict, const char *address_ip, int port_ip, int protocol);

IL_EXPORT int il_servo_is_connected(il_net_t **net, const char *address_ip, int port_ip, int protocol);

/**
 * Obtain the number of axis of the servo.
 *
 * @param [in] servo
 *	Servo instance.
 *
 * @return
 *	Number of axis available.
 *
 */
IL_EXPORT const uint16_t *il_servo_subnodes_get(il_servo_t *servo);

IL_EXPORT int il_servo_connect_ecat(il_net_prot_t prot, char *ifname, char *if_address_ip, il_net_t **net, il_servo_t **servo,
		   const char *dict, const char *address_ip, int port_ip, uint16_t slave);

IL_END_DECL

#endif
