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

#ifndef PUBLIC_INGENIALINK_REGISTERS_H_
#define PUBLIC_INGENIALINK_REGISTERS_H_

#include "common.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/registers.h
 * @brief Registers.
 * @defgroup IL_REGS Registers
 * @ingroup IL
 * @{
 */

/** IngeniaLink register data type. */
typedef enum {
	/** Raw data. */
	IL_REG_DTYPE_RAW,
	/** Unsigned 8-bit integer. */
	IL_REG_DTYPE_U8,
	/** Signed 8-bit integer. */
	IL_REG_DTYPE_S8,
	/** Unsigned 16-bit integer. */
	IL_REG_DTYPE_U16,
	/** Signed 16-bit integer. */
	IL_REG_DTYPE_S16,
	/** Unsigned 32-bit integer. */
	IL_REG_DTYPE_U32,
	/** Signed 32-bit integer. */
	IL_REG_DTYPE_S32,
	/** Unsigned 64-bit integer. */
	IL_REG_DTYPE_U64,
	/** Signed 64-bit integer. */
	IL_REG_DTYPE_S64,
} il_reg_dtype_t;

/** IngeniaLink register access. */
typedef enum {
	/** Read/Write. */
	IL_REG_ACCESS_RW,
	/** Read only. */
	IL_REG_ACCESS_RO,
	/** Write only */
	IL_REG_ACCESS_WO,
} il_reg_access_t;

/** IngeniaLink register physical units type. */
typedef enum {
	/** None. */
	IL_REG_PHY_NONE,
	/** Torque. */
	IL_REG_PHY_TORQUE,
	/** Position. */
	IL_REG_PHY_POS,
	/** Velocity. */
	IL_REG_PHY_VEL,
	/** Acceleration. */
	IL_REG_PHY_ACC,
	/** Voltage (relative to DC bus). */
	IL_REG_PHY_VOLT_REL,
	/** Radians. */
	IL_REG_PHY_RAD,
} il_reg_phy_t;

/** IngeniaLink register. */
typedef struct {
	/** Address. */
	uint32_t address;
	/** Data type. */
	il_reg_dtype_t dtype;
	/** Access type. */
	il_reg_access_t access;
	/** Physical units type. */
	il_reg_phy_t phy;
} il_reg_t;

/**
 * @defgroup IL_REGS_CIA301 CiA 301
 * @{
 */

/** Software version. */
IL_EXPORT extern const il_reg_t IL_REG_SW_VERSION;

/** Store parameters: save all. */
IL_EXPORT extern const il_reg_t IL_REG_STORE_ALL;

/** Store parameters: communications. */
IL_EXPORT extern const il_reg_t IL_REG_STORE_COMM;

/** Store parameters: application. */
IL_EXPORT extern const il_reg_t IL_REG_STORE_APP;

/** Identity object: vendor ID. */
IL_EXPORT extern const il_reg_t IL_REG_ID_VID;

/** Identity object: product code. */
IL_EXPORT extern const il_reg_t IL_REG_ID_PROD_CODE;

/** Identity object: revision number. */
IL_EXPORT extern const il_reg_t IL_REG_ID_REVISION;

/** Identity object: serial number. */
IL_EXPORT extern const il_reg_t IL_REG_ID_SERIAL;

/** @} */

/**
 * @defgroup IL_REGS_VENDOR Vendor specific (Ingenia)
 * @{
 */

/** Motor pair poles. */
IL_EXPORT extern const il_reg_t IL_REG_PAIR_POLES;

/** Commutation sensor. */
IL_EXPORT extern const il_reg_t IL_REG_COMMUTATION_SENSOR;

/** Commutation: initial angle determination method. */
IL_EXPORT extern const il_reg_t IL_REG_COMMUTATION_IANGLE_METHOD;

/** Commutation: actual system angle. */
IL_EXPORT extern const il_reg_t IL_REG_COMMUTATION_ANGLE_ACT;

/** Commutation: reference sensor. */
IL_EXPORT extern const il_reg_t IL_REG_COMMUTATION_REF_SENSOR;

/** Commutation: angle offset. */
IL_EXPORT extern const il_reg_t IL_REG_COMMUTATION_ANGLE_OFF;

/** Non-incremental alignment: offset from phase A. */
IL_EXPORT extern const il_reg_t IL_REG_NINCR_ALIGN_OFFSET_PHASE_A;

/** Feedbacks: torque sensor. */
IL_EXPORT extern const il_reg_t IL_REG_FB_TORQUE_SENSOR;

/** Feedbacks: velocity sensor. */
IL_EXPORT extern const il_reg_t IL_REG_FB_VEL_SENSOR;

/** Feedbacks: position sensor. */
IL_EXPORT extern const il_reg_t IL_REG_FB_POS_SENSOR;

/** Digital encoder/SinCos swap mode. */
IL_EXPORT extern const il_reg_t IL_REG_DENC_SINCOS_SWAP;

/** Digital encoder/SinCos type. */
IL_EXPORT extern const il_reg_t IL_REG_DENC_SINCOS_TYPE;

/** Digital encoder/SinCos glitch filter. */
IL_EXPORT extern const il_reg_t IL_REG_DENC_SINCOS_FILTER;

/** SSI: single-turn bits. */
IL_EXPORT extern const il_reg_t IL_REG_SSI_STURNBITS;

/** Command reference source. */
IL_EXPORT extern const il_reg_t IL_REG_CMD_SRC;

/** Torque demand low pass filter: enabled. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_DEM_LPF_ENABLED;

/** Torque demand low pass filter: cutoff frequency. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_DEM_LPF_CFREQ;

/** Control loops configuration: bypass torque loop. */
IL_EXPORT extern const il_reg_t IL_REG_CL_TORQUE_BYPASS;

/** Control loops configuration: position feedback openloop. */
IL_EXPORT extern const il_reg_t IL_REG_CL_FB_POS_OPEN;

/** Control loops configuration: velocity feedback openloop. */
IL_EXPORT extern const il_reg_t IL_REG_CL_FB_VEL_OPEN;

/** Control loops configuration: torque feedback openloop. */
IL_EXPORT extern const il_reg_t IL_REG_CL_FB_TORQUE_OPEN;

/** Control loops configuration: velocity mode use position loop. */
IL_EXPORT extern const il_reg_t IL_REG_CL_VEL_USE_POS;

/** Torque window. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_WINDOW;

/** Torque window time (0.1 ms). */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_WINDOW_TIME;

/** Disturbance signal: maximum entries. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_MAX_ENTRIES;

/** Disturbance signal: filled entries. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_FILLED_ENTRIES;

/** Disturbance signal: entry number. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_ENTRY_NUM;

/** Disturbance signal: entry value. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_ENTRY_VAL;

/** Disturbance signal: injection point. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_INJ_POINT;

/** Disturbance signal: number of cycles. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_NCYCLES;

/** Disturbance signal: injection rate. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_RATE;

/** Disturbance signal: output signal. */
IL_EXPORT extern const il_reg_t IL_REG_DSIG_OUT;

/** Motor parameters: resistance phase-to-phase. */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_RES_PHTOPH;

/** Motor parameters: inductance phase-to-phase. */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_IND_PHTOPH;

/** Motor parameters: pole pitch. */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_PPITCH;

/** Motor parameters: motor backemf constant (Kv). */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_KV;

/** Motor parameters: stroke. */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_STROKE;

/** Motor parameters: torque constant (Km). */
IL_EXPORT extern const il_reg_t IL_REG_MOTPARAM_KM;

/** Monitor config: sampling rage (period). */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_CFG_T_S;

/** Monitor config: enable mode. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_CFG_ENABLE;

/** Monitor config: trigger delay in samples. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_CFG_DELAY_SAMPLES;

/** Monitor result: max entry number. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_SZ;

/** Monitor result: filled entry values. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_FILLED;

/** Monitor result: filled entry values. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_ENTRY;

/** Monitor result: actual entry 1. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_CH_1;

/** Monitor result: actual entry 2. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_CH_2;

/** Monitor result: actual entry 3. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_CH_3;

/** Monitor result: actual entry 4. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_RESULT_CH_4;

/** Monitor mapping: channel 1. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_MAP_CH_1;

/** Monitor mapping: channel 2. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_MAP_CH_2;

/** Monitor mapping: channel 3. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_MAP_CH_3;

/** Monitor mapping: channel 4. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_MAP_CH_4;

/** Monitor trigger, mode. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_MODE;

/** Monitor trigger, source register. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_SRC;

/** Monitor trigger, positive threshold. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_TH_POS;

/** Monitor trigger, negative threshold. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_TH_NEG;

/** Monitor trigger, digital input mask. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_DIN_MSK;

/** Monitor trigger, delay in samples. */
IL_EXPORT extern const il_reg_t IL_REG_MONITOR_TRIG_DELAY;

/** Open loop parameters, target voltage. */
IL_EXPORT extern const il_reg_t IL_REG_OL_VOLTAGE;

/** Open loop parameters, target frequency. */
IL_EXPORT extern const il_reg_t IL_REG_OL_FREQUENCY;

/** Hardware config, current sensing resistor value (10 uOhm). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_SENSING_RES;

/** Hardware config, preamplifier gain. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PREAMP_G;

/** Hardware config, has variable gain amplifier. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_VGA;

/** Hardware config, has temperature sensor. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_T_SENSOR;

/** Hardware config, maximum absolute temperature (mC). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_T_MAX;

/** Hardware config, minimum absolute temperature (mC). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_T_MIN;

/** Hardware config, has Vbus sensor. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_VBUS_SENSOR;

/** Hardware config, maximum absolute voltage (mV). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_V_MAX;

/** Hardware config, minimum absolute voltage (mV). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_V_MIN;

/** Hardware config, nominal current (mA, RMS). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_NOM;

/** Hardware config, peak current (mA, RMS). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_PEAK;

/** Hardware config, maximum peak time (100 us). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PEAK_TIME_MAX;

/** Hardware config, maximum peak current (mA, RMS). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_PEAK_MAX;

/** Hardware config, stepper in 3-ph available. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_STEPPER_3PH;

/** Hardware config, has NVM. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_NVM;

/** Hardware config, hardware error available. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_HW_ERR;

/** Hardware config, temperature sensor offset (mV). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_T_SENSOR_OFFSET;

/** Hardware config, temperature sensor gain (mV/deg). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_T_SENSOR_GAIN;

/** Hardware config, Vbus sensor gain. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_VBUS_SENSOR_GAIN;

/** Hardware config, deadtime (ns). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_DEADTIME;

/** Hardware config, PWM frequency (legacy). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PWM_FREQ_LEGACY;

/** Hardware config, bootstrap charge time (ms). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_BOOTSTRAP_CHRG_TIME;

/** Hardware config, has NTC. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_NTC;

/** Hardware config, NTC Rext (Ohm). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_NTC_REXT;

/** Hardware config, NTC resistance @25 deg (Ohm). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_NTC_R25;

/** Hardware config, NTC B parameter @25 deg (K). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_NTC_B25;

/** Hardware config, has resolver. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HAS_RESOLVER;

/** Hardware config, hardware node id. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_HW_NODEID;

/** Hardware config, PWM frequency (Hz). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PWM_FREQ;

/** Hardware config, available digital inputs. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_DIN_AVAIL;

/** Hardware config, available digital outputs. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_DOUT_AVAIL;

/** Hardware config, available analog inputs. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AIN_AVAIL;

/** Hardware config, available analog outputs. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AOUT_AVAIL;

/** Hardware config, product id. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PROD_ID;

/** Hardware config, serial number. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_SERIAL;

/** Hardware config, two-phases switching scheme. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_2PH_SW_SCHEME;

/** Hardware config, driver enable in UART com. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_ENA_IN_UART;

/** Hardware config, available current sensors. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_SENSORS_AVAIL;

/** Hardware config, digital input polarity mask. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_DIN_POL_MSK;

/** Hardware config, digital output polarity mask. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_DOUT_POL_MSK;

/** Hardware config, analog reference. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AREF;

/** Hardware config, analog input 1-2 parameters. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AIN12_PARAM;

/** Hardware config, analog input 3-4 parameters. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AIN34_PARAM;

/** Hardware config, analog output 1-2 parameters. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AOUT12_PARAM;

/** Hardware config, macro parameters. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_MACRO_PARAM;

/** Hardware config, supported motor types. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_MOTOR_SUP;

/** Hardware config, supported communications. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_COMMS_SUP;

/** Hardware config, supported feedbacks. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_FEEDBACKS_SUP;

/** Hardware config, PWM maximum duty cycle. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PWM_DUTY_MAX;

/** Hardware config, current loop frequency (Hz). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_LOOP_FREQ;

/** Hardware config, velocity/position loops frequency (Hz). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_VELPOS_LOOP_FREQ;

/** Hardware config, supported command sources. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CMDSRC_SUP;

/** Hardware config, STO GPI input. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_STO_IN;

/** Hardware config, alternative PWM frequency (Hz). */
IL_EXPORT extern const il_reg_t IL_REG_HWC_PWM_FREQ_ALT;

/** Hardware config, current low pass filter output. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_CURR_LPF_OUT;

/** Hardware config, analog input 5 parameters. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_AIN5_PARAM;

/** Hardware config, open load protection input. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_OLP_IN;

/** Hardware config, shunt output. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_SHUNT_OUT;

/** Hardware config, brake output. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_BRAKE_OUT;

/** Hardware config, key. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_KEY;

/** Hardware config, number of available registers . */
IL_EXPORT extern const il_reg_t IL_REG_HWC_REG_CNT;

/** Hardware version. */
IL_EXPORT extern const il_reg_t IL_REG_HW_VERSION;

/** Hardware variant. */
IL_EXPORT extern const il_reg_t IL_REG_HW_VARIANT;

/** Programming date. */
IL_EXPORT extern const il_reg_t IL_REG_PROG_DATE;

/** Hardware config revision. */
IL_EXPORT extern const il_reg_t IL_REG_HWC_REVISION;

/** Drive name. */
IL_EXPORT extern const il_reg_t IL_REG_DRIVE_NAME;

/** Reset device. */
IL_EXPORT extern const il_reg_t IL_REG_RESET;

/** @} */

/**
 * @defgroup IL_REGS_CIA402 CiA 402
 * @{
 */

/** Control word. */
IL_EXPORT extern const il_reg_t IL_REG_CTL_WORD;

/** Status word. */
IL_EXPORT extern const il_reg_t IL_REG_STS_WORD;

/** Operation mode. */
IL_EXPORT extern const il_reg_t IL_REG_OP_MODE;

/** Operation mode display. */
IL_EXPORT extern const il_reg_t IL_REG_OP_MODE_DISP;

/** Position actual value. */
IL_EXPORT extern const il_reg_t IL_REG_POS_ACT;

/** Velocity actual value. */
IL_EXPORT extern const il_reg_t IL_REG_VEL_ACT;

/** Target torque. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_TGT;

/** Maximum torque. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_MAX;

/** Motor rated current. */
IL_EXPORT extern const il_reg_t IL_REG_RATED_CURRENT;

/** Motor rated torque. */
IL_EXPORT extern const il_reg_t IL_REG_RATED_TORQUE;

/** Torque actual value. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_ACT;

/** DC link circuit voltage. */
IL_EXPORT extern const il_reg_t IL_REG_DC_VOLTAGE;

/** Target position. */
IL_EXPORT extern const il_reg_t IL_REG_POS_TGT;

/** Torque slope. */
IL_EXPORT extern const il_reg_t IL_REG_TORQUE_SLOPE;

/** Position encoder resolution: encoder increments. */
IL_EXPORT extern const il_reg_t IL_REG_PRES_ENC_INCR;

/** Position encoder resolution: motor revolutions. */
IL_EXPORT extern const il_reg_t IL_REG_PRES_MOTOR_REVS;

/** Velocity encoder resolution: encoder increments. */
IL_EXPORT extern const il_reg_t IL_REG_VRES_ENC_INCR;

/** Velocity encoder resolution: motor revolutions. */
IL_EXPORT extern const il_reg_t IL_REG_VRES_MOTOR_REVS;

/** Target velocity. */
IL_EXPORT extern const il_reg_t IL_REG_VEL_TGT;

/** Motor type. */
IL_EXPORT extern const il_reg_t IL_REG_MOTOR_TYPE;

/** @} */

/** @} */

IL_END_DECL

#endif
