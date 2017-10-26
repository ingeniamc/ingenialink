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
	/** Index */
	uint16_t idx;
	/** Subindex */
	uint8_t sidx;
	/** Data type. */
	il_reg_dtype_t dtype;
	/** Access type. */
	il_reg_access_t access;
	/** Physical units type. */
	il_reg_phy_t phy;
} il_reg_t;

/*
 * Vendor specific (Ingenia)
 */

/** Motor pair poles. */
extern const il_reg_t IL_REG_PAIR_POLES;

/** Commutation sensor. */
extern const il_reg_t IL_REG_COMMUTATION_SENSOR;

/** Commutation: initial angle determination method. */
extern const il_reg_t IL_REG_COMMUTATION_IANGLE_METHOD;

/** Commutation: actual system angle. */
extern const il_reg_t IL_REG_COMMUTATION_ANGLE_ACT;

/** Commutation: reference sensor. */
extern const il_reg_t IL_REG_COMMUTATION_REF_SENSOR;

/** Commutation: angle offset. */
extern const il_reg_t IL_REG_COMMUTATION_ANGLE_OFF;

/** Non-incremental alignment: offset from phase A. */
extern const il_reg_t IL_REG_NINCR_ALIGN_OFFSET_PHASE_A;

/** Feedbacks: torque sensor. */
extern const il_reg_t IL_REG_FB_TORQUE_SENSOR;

/** Feedbacks: velocity sensor. */
extern const il_reg_t IL_REG_FB_VEL_SENSOR;

/** Feedbacks: position sensor. */
extern const il_reg_t IL_REG_FB_POS_SENSOR;

/** Digital encoder/SinCos swap mode. */
extern const il_reg_t IL_REG_DENC_SINCOS_SWAP;

/** Digital encoder/SinCos type. */
extern const il_reg_t IL_REG_DENC_SINCOS_TYPE;

/** Digital encoder/SinCos glitch filter. */
extern const il_reg_t IL_REG_DENC_SINCOS_FILTER;

/** SSI: single-turn bits. */
extern const il_reg_t IL_REG_SSI_STURNBITS;

/** Control loops configuration: bypass torque loop. */
extern const il_reg_t IL_REG_CL_TORQUE_BYPASS;

/** Control loops configuration: position feedback openloop. */
extern const il_reg_t IL_REG_CL_FB_POS_OPEN;

/** Control loops configuration: velocity feedback openloop. */
extern const il_reg_t IL_REG_CL_FB_VEL_OPEN;

/** Control loops configuration: torque feedback openloop. */
extern const il_reg_t IL_REG_CL_FB_TORQUE_OPEN;

/** Control loops configuration: velocity mode use position loop. */
extern const il_reg_t IL_REG_CL_VEL_USE_POS;

/** Torque window. */
extern const il_reg_t IL_REG_TORQUE_WINDOW;

/** Torque window time (0.1 ms). */
extern const il_reg_t IL_REG_TORQUE_WINDOW_TIME;

/** Disturbance signal: maximum entries. */
extern const il_reg_t IL_REG_DSIG_MAX_ENTRIES;

/** Disturbance signal: filled entries. */
extern const il_reg_t IL_REG_DSIG_FILLED_ENTRIES;

/** Disturbance signal: entry number. */
extern const il_reg_t IL_REG_DSIG_ENTRY_NUM;

/** Disturbance signal: entry value. */
extern const il_reg_t IL_REG_DSIG_ENTRY_VAL;

/** Disturbance signal: injection point. */
extern const il_reg_t IL_REG_DSIG_INJ_POINT;

/** Disturbance signal: number of cycles. */
extern const il_reg_t IL_REG_DSIG_NCYCLES;

/** Disturbance signal: injection rate. */
extern const il_reg_t IL_REG_DSIG_RATE;

/** Disturbance signal: output signal. */
extern const il_reg_t IL_REG_DSIG_OUT;

/** Motor parameters: pole pitch. */
extern const il_reg_t IL_REG_MOTPARAM_PPITCH;

/** Monitor config: sampling rage (period). */
extern const il_reg_t IL_REG_MONITOR_CFG_T_S;

/** Monitor config: enable mode. */
extern const il_reg_t IL_REG_MONITOR_CFG_ENABLE;

/** Monitor config: trigger delay in samples. */
extern const il_reg_t IL_REG_MONITOR_CFG_DELAY_SAMPLES;

/** Monitor result: max entry number. */
extern const il_reg_t IL_REG_MONITOR_RESULT_SZ;

/** Monitor result: filled entry values. */
extern const il_reg_t IL_REG_MONITOR_RESULT_FILLED;

/** Monitor result: filled entry values. */
extern const il_reg_t IL_REG_MONITOR_RESULT_ENTRY;

/** Monitor result: actual entry 1. */
extern const il_reg_t IL_REG_MONITOR_RESULT_CH_1;

/** Monitor result: actual entry 2. */
extern const il_reg_t IL_REG_MONITOR_RESULT_CH_2;

/** Monitor result: actual entry 3. */
extern const il_reg_t IL_REG_MONITOR_RESULT_CH_3;

/** Monitor result: actual entry 4. */
extern const il_reg_t IL_REG_MONITOR_RESULT_CH_4;

/** Monitor mapping: channel 1. */
extern const il_reg_t IL_REG_MONITOR_MAP_CH_1;

/** Monitor mapping: channel 2. */
extern const il_reg_t IL_REG_MONITOR_MAP_CH_2;

/** Monitor mapping: channel 3. */
extern const il_reg_t IL_REG_MONITOR_MAP_CH_3;

/** Monitor mapping: channel 4. */
extern const il_reg_t IL_REG_MONITOR_MAP_CH_4;

/** Monitor trigger, mode. */
extern const il_reg_t IL_REG_MONITOR_TRIG_MODE;

/** Monitor trigger, source register. */
extern const il_reg_t IL_REG_MONITOR_TRIG_SRC;

/** Monitor trigger, positive threshold. */
extern const il_reg_t IL_REG_MONITOR_TRIG_TH_POS;

/** Monitor trigger, negative threshold. */
extern const il_reg_t IL_REG_MONITOR_TRIG_TH_NEG;

/** Monitor trigger, digital input mask. */
extern const il_reg_t IL_REG_MONITOR_TRIG_DIN_MSK;

/** Monitor trigger, delay in samples. */
extern const il_reg_t IL_REG_MONITOR_TRIG_DELAY;

/** Open loop parameters, target voltage. */
extern const il_reg_t IL_REG_OL_VOLTAGE;

/** Open loop parameters, target frequency. */
extern const il_reg_t IL_REG_OL_FREQUENCY;

/** Hardware config, current sensing resistor value (10 uOhm). */
extern const il_reg_t IL_REG_HWC_SENSING_RES;

/** Hardware config, preamplifier gain. */
extern const il_reg_t IL_REG_HWC_PREAMP_G;

/** Hardware config, has variable gain amplifier. */
extern const il_reg_t IL_REG_HWC_HAS_VGA;

/** Hardware config, has temperature sensor. */
extern const il_reg_t IL_REG_HWC_HAS_T_SENSOR;

/** Hardware config, maximum absolute temperature (mC). */
extern const il_reg_t IL_REG_HWC_T_MAX;

/** Hardware config, minimum absolute temperature (mC). */
extern const il_reg_t IL_REG_HWC_T_MIN;

/** Hardware config, has Vbus sensor. */
extern const il_reg_t IL_REG_HWC_HAS_VBUS_SENSOR;

/** Hardware config, maximum absolute voltage (mV). */
extern const il_reg_t IL_REG_HWC_V_MAX;

/** Hardware config, minimum absolute voltage (mV). */
extern const il_reg_t IL_REG_HWC_V_MIN;

/** Hardware config, nominal current (mA, RMS). */
extern const il_reg_t IL_REG_HWC_CURR_NOM;

/** Hardware config, peak current (mA, RMS). */
extern const il_reg_t IL_REG_HWC_CURR_PEAK;

/** Hardware config, maximum peak time (100 us). */
extern const il_reg_t IL_REG_HWC_PEAK_TIME_MAX;

/** Hardware config, maximum peak current (mA, RMS). */
extern const il_reg_t IL_REG_HWC_CURR_PEAK_MAX;

/** Hardware config, stepper in 3-ph available. */
extern const il_reg_t IL_REG_HWC_HAS_STEPPER_3PH;

/** Hardware config, has NVM. */
extern const il_reg_t IL_REG_HWC_HAS_NVM;

/** Hardware config, hardware error available. */
extern const il_reg_t IL_REG_HWC_HAS_HW_ERR;

/** Hardware config, temperature sensor offset (mV). */
extern const il_reg_t IL_REG_HWC_T_SENSOR_OFFSET;

/** Hardware config, temperature sensor gain (mV/deg). */
extern const il_reg_t IL_REG_HWC_T_SENSOR_GAIN;

/** Hardware config, Vbus sensor gain. */
extern const il_reg_t IL_REG_HWC_VBUS_SENSOR_GAIN;

/** Hardware config, deadtime (ns). */
extern const il_reg_t IL_REG_HWC_DEADTIME;

/** Hardware config, PWM frequency (legacy). */
extern const il_reg_t IL_REG_HWC_PWM_FREQ_LEGACY;

/** Hardware config, bootstrap charge time (ms). */
extern const il_reg_t IL_REG_HWC_BOOTSTRAP_CHRG_TIME;

/** Hardware config, has NTC. */
extern const il_reg_t IL_REG_HWC_HAS_NTC;

/** Hardware config, NTC Rext (Ohm). */
extern const il_reg_t IL_REG_HWC_NTC_REXT;

/** Hardware config, NTC resistance @25 deg (Ohm). */
extern const il_reg_t IL_REG_HWC_NTC_R25;

/** Hardware config, NTC B parameter @25 deg (K). */
extern const il_reg_t IL_REG_HWC_NTC_B25;

/** Hardware config, has resolver. */
extern const il_reg_t IL_REG_HWC_HAS_RESOLVER;

/** Hardware config, hardware node id. */
extern const il_reg_t IL_REG_HWC_HW_NODEID;

/** Hardware config, PWM frequency (Hz). */
extern const il_reg_t IL_REG_HWC_PWM_FREQ;

/** Hardware config, available digital inputs. */
extern const il_reg_t IL_REG_HWC_DIN_AVAIL;

/** Hardware config, available digital outputs. */
extern const il_reg_t IL_REG_HWC_DOUT_AVAIL;

/** Hardware config, available analog inputs. */
extern const il_reg_t IL_REG_HWC_AIN_AVAIL;

/** Hardware config, available analog outputs. */
extern const il_reg_t IL_REG_HWC_AOUT_AVAIL;

/** Hardware config, product id. */
extern const il_reg_t IL_REG_HWC_PROD_ID;

/** Hardware config, serial number. */
extern const il_reg_t IL_REG_HWC_SERIAL;

/** Hardware config, two-phases switching scheme. */
extern const il_reg_t IL_REG_HWC_2PH_SW_SCHEME;

/** Hardware config, driver enable in UART com. */
extern const il_reg_t IL_REG_HWC_ENA_IN_UART;

/** Hardware config, available current sensors. */
extern const il_reg_t IL_REG_HWC_CURR_SENSORS_AVAIL;

/** Hardware config, digital input polarity mask. */
extern const il_reg_t IL_REG_HWC_DIN_POL_MSK;

/** Hardware config, digital output polarity mask. */
extern const il_reg_t IL_REG_HWC_DOUT_POL_MSK;

/** Hardware config, analog reference. */
extern const il_reg_t IL_REG_HWC_AREF;

/** Hardware config, analog input 1-2 parameters. */
extern const il_reg_t IL_REG_HWC_AIN12_PARAM;

/** Hardware config, analog input 3-4 parameters. */
extern const il_reg_t IL_REG_HWC_AIN34_PARAM;

/** Hardware config, analog output 1-2 parameters. */
extern const il_reg_t IL_REG_HWC_AOUT12_PARAM;

/** Hardware config, macro parameters. */
extern const il_reg_t IL_REG_HWC_MACRO_PARAM;

/** Hardware config, supported motor types. */
extern const il_reg_t IL_REG_HWC_MOTOR_SUP;

/** Hardware config, supported communications. */
extern const il_reg_t IL_REG_HWC_COMMS_SUP;

/** Hardware config, supported feedbacks. */
extern const il_reg_t IL_REG_HWC_FEEDBACKS_SUP;

/** Hardware config, PWM maximum duty cycle. */
extern const il_reg_t IL_REG_HWC_PWM_DUTY_MAX;

/** Hardware config, current loop frequency (Hz). */
extern const il_reg_t IL_REG_HWC_CURR_LOOP_FREQ;

/** Hardware config, velocity/position loops frequency (Hz). */
extern const il_reg_t IL_REG_HWC_VELPOS_LOOP_FREQ;

/** Hardware config, supported command sources. */
extern const il_reg_t IL_REG_HWC_CMDSRC_SUP;

/** Hardware config, STO GPI input. */
extern const il_reg_t IL_REG_HWC_STO_IN;

/** Hardware config, alternative PWM frequency (Hz). */
extern const il_reg_t IL_REG_HWC_PWM_FREQ_ALT;

/** Hardware config, current low pass filter output. */
extern const il_reg_t IL_REG_HWC_CURR_LPF_OUT;

/** Hardware config, analog input 5 parameters. */
extern const il_reg_t IL_REG_HWC_AIN5_PARAM;

/** Hardware config, open load protection input. */
extern const il_reg_t IL_REG_HWC_OLP_IN;

/** Hardware config, shunt output. */
extern const il_reg_t IL_REG_HWC_SHUNT_OUT;

/** Hardware config, brake output. */
extern const il_reg_t IL_REG_HWC_BRAKE_OUT;

/** Hardware config, key. */
extern const il_reg_t IL_REG_HWC_KEY;

/** Hardware config, number of available registers . */
extern const il_reg_t IL_REG_HWC_REG_CNT;

/** Hardware version. */
extern const il_reg_t IL_REG_HW_VERSION;

/** Hardware variant. */
extern const il_reg_t IL_REG_HW_VARIANT;

/** Programming date. */
extern const il_reg_t IL_REG_PROG_DATE;

/** Hardware config revision. */
extern const il_reg_t IL_REG_HWC_REVISION;

/*
 * CiA 402
 */

/** Control word. */
extern const il_reg_t IL_REG_CTL_WORD;

/** Status word. */
extern const il_reg_t IL_REG_STS_WORD;

/** Operation mode. */
extern const il_reg_t IL_REG_OP_MODE;

/** Position actual value. */
extern const il_reg_t IL_REG_POS_ACT;

/** Velocity actual value. */
extern const il_reg_t IL_REG_VEL_ACT;

/** Target torque. */
extern const il_reg_t IL_REG_TORQUE_TGT;

/** Motor rated torque. */
extern const il_reg_t IL_REG_RATED_TORQUE;

/** Torque actual value. */
extern const il_reg_t IL_REG_TORQUE_ACT;

/** DC link circuit voltage. */
extern const il_reg_t IL_REG_DC_VOLTAGE;

/** Target position. */
extern const il_reg_t IL_REG_POS_TGT;

/** Torque slope. */
extern const il_reg_t IL_REG_TORQUE_SLOPE;

/** Position encoder resolution: encoder increments. */
extern const il_reg_t IL_REG_PRES_ENC_INCR;

/** Position encoder resolution: motor revolutions. */
extern const il_reg_t IL_REG_PRES_MOTOR_REVS;

/** Velocity encoder resolution: encoder increments. */
extern const il_reg_t IL_REG_VRES_ENC_INCR;

/** Velocity encoder resolution: motor revolutions. */
extern const il_reg_t IL_REG_VRES_MOTOR_REVS;

/** Target velocity. */
extern const il_reg_t IL_REG_VEL_TGT;

/** Motor type. */
extern const il_reg_t IL_REG_MOTOR_TYPE;

/** @} */

IL_END_DECL

#endif
