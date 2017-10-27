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

#ifndef PUBLIC_INGENIALINK_CONST_H_
#define PUBLIC_INGENIALINK_CONST_H_

/**
 * @file ingenialink/const.h
 * @brief Constants.
 * @defgroup IL_CONST Constants
 * @ingroup IL
 * @{
 */

/**
 * @defgroup IL_CONST_SIGNATURES Signatures
 * @{
 */

/** Store signature. */
#define ILK_SIGNATURE_STORE	0x65766173

/** Reset signature. */
#define ILK_SIGNATURE_RESET	0x64747372

/** @} */

/**
 * @defgroup IL_CONST_OP_MODE Operation modes
 * @{
 */

/** Open loop (vector mode). */
#define ILK_OP_MODE_OLV		-2
/** Open loop (scalar mode). */
#define ILK_OP_MODE_OLS		-1
/** Profile position. */
#define ILK_OP_MODE_PP		1
/** Velocity. */
#define ILK_OP_MODE_VEL		2
/** Profile velocity. */
#define ILK_OP_MODE_PV		3
/** Profile torque. */
#define ILK_OP_MODE_PT		4
/** Homing. */
#define ILK_OP_MODE_HOMING	6
/** Interpolated position mode. */
#define ILK_OP_MODE_IP		7
/** Cyclic sync position mode. */
#define ILK_OP_MODE_CSP		8
/** Cyclic sync velocity mode. */
#define ILK_OP_MODE_CSV		9
/** Cyclic sync torque mode. */
#define ILK_OP_MODE_CST		10

/** @} */

/**
 * @defgroup IL_CONST_MOTOR Motor types
 * @{
 */

/** Squirrel cage induction. */
#define ILK_MOTOR_SQUIRREL	0x0007
/** Stepper with microstepping capability. */
#define ILK_MOTOR_STEPPER	0x0009
/** Rotary brushless AC. */
#define ILK_MOTOR_ROT_BLAC	0x000A
/** Rotary brushless DC. */
#define ILK_MOTOR_ROT_BLDC	0x000B
/** Rotary DC. */
#define ILK_MOTOR_ROT_DC	0x000D
/** Rotary voice coil. */
#define ILK_MOTOR_ROT_VC	0x8000
/** Linear brushless AC. */
#define ILK_MOTOR_LIN_BLAC	0x8001
/** Linear brushless DC. */
#define ILK_MOTOR_LIN_BLDC	0x8002
/** Linear voice coil. */
#define ILK_MOTOR_LIN_VC	0x8003
/** Linear DC */
#define ILK_MOTOR_LIN_DC	0x8004

/** @} */

/**
 * @defgroup IL_CONST_FB Feedbacks
 * @{
 */

/** Digital encoder. */
#define ILK_FB_DIGITAL_ENCODER	0x00
/** Digital halls. */
#define ILK_FB_DIGITAL_HALLS	0x01
/** Analog halls. */
#define ILK_FB_ANALOG_HALLS	0x02
/** Analog input. */
#define ILK_FB_ANALOG_INPUT	0x04
/** SSI. */
#define ILK_FB_SSI		0x05
/** SinCos. */
#define ILK_FB_SINCOS		0x06
/** PWM. */
#define ILK_FB_PWM		0x07
/** Resolver. */
#define ILK_FB_RESOLVER		0x08
/** None. */
#define ILK_FB_NONE		0x09
/** Simulated. */
#define ILK_FB_SIMULATED	0x0B

/** @} */

/**
 * @defgroup IL_CONST_VEL_SENSOR Velocity sensors
 * @{
 */

/** Position sensor. */
#define ILK_VEL_SENSOR_POS		0x00
/** External DC tachometer. */
#define ILK_VEL_SENSOR_TACHOMETER	0x01
/** None. */
#define ILK_VEL_SENSOR_NONE		0x02
/** Digital halls. */
#define ILK_VEL_SENSOR_DIGITAL_HALLS	0x04

/** @} */

/**
 * @defgroup IL_CONST_POS_SENSOR Position sensors
 * @{
 */

/** Digital encoder. */
#define ILK_POS_SENSOR_DIGITAL_ENCODER	0x00
/** Digital halls. */
#define ILK_POS_SENSOR_DIGITAL_HALLS	0x01
/** Analog halls. */
#define ILK_POS_SENSOR_ANALOG_HALLS	0x02
/** Analog input. */
#define ILK_POS_SENSOR_ANALOG_INPUT	0x04
/** SSI. */
#define ILK_POS_SENSOR_SSI		0x05
/** SinCos. */
#define ILK_POS_SENSOR_SINCOS		0x06
/** PWM. */
#define ILK_POS_SENSOR_PWM		0x07
/** Resolver. */
#define ILK_POS_SENSOR_RESOLVER		0x08
/** None. */
#define ILK_POS_SENSOR_NONE		0x09
/** Simulated. */
#define ILK_POS_SENSOR_SIMULATED	0x0B
/** Digital tachometer. */
#define ILK_POS_SENSOR_TACHOMETER	0x0C

/** @} */

/**
 * @defgroup IL_CONST_IANGLE_METHOD Initial angle determination methods
 * @{
 */

/** Forced alignment method. */
#define ILK_IANGLE_METHOD_FORCED_ALIGN	0x00
/** Non incremental sensor used. */
#define ILK_IANGLE_METHOD_NINCR_SENSOR	0x01
/** Initial rotor position known. */
#define ILK_IANGLE_METHOD_POS_KNOWN	0x02

/** @} */

/**
 * @defgroup IL_CONST_DENC_SINCOS_PARAM Digital encoder / SinCos parameters
 * @{
 */

/** Not swapped. */
#define ILK_DENC_SINCOS_NSWAPPED		0x00
/** Swapped. */
#define ILK_DENC_SINCOS_SWAPPED			0x01

/** No encoder. */
#define ILK_DENC_SINCOS_TYPE_NONE		0x00
/** 2 channels encoder (single ended). */
#define ILK_DENC_SINCOS_TYPE_2CH_S		0x01
/** 2 channels + index encoder (single ended). */
#define ILK_DENC_SINCOS_TYPE_2CH_S_IDX		0x02
/** 2 channels encoder (differential). */
#define ILK_DENC_SINCOS_TYPE_2CH_D		0x03
/** 2 channels + index encoder (differential). */
#define ILK_DENC_SINCOS_TYPE_2CH_D_IDX		0x04

/** Maximum encoder frequency, 30 MHz. */
#define ILK_DENC_SINCOS_FILTER_30MHZ		0x00
/** Maximum encoder frequency, 10 MHz. */
#define ILK_DENC_SINCOS_FILTER_10MHZ		0x01
/** Maximum encoder frequency, 5 MHz. */
#define ILK_DENC_SINCOS_FILTER_5MHZ		0x02
/** Maximum encoder frequency, 2 MHz. */
#define ILK_DENC_SINCOS_FILTER_2MHZ		0x03
/** Maximum encoder frequency, 1 MHz. */
#define ILK_DENC_SINCOS_FILTER_1MHZ		0x04
/** Maximum encoder frequency, 625 KHz. */
#define ILK_DENC_SINCOS_FILTER_625KHZ		0x05
/** Maximum encoder frequency, 312,5 KHz. */
#define ILK_DENC_SINCOS_FILTER_312_5KHZ		0x06
/** Maximum encoder frequency, 156,25 KHz. */
#define ILK_DENC_SINCOS_FILTER_156_25KHZ	0x07
/** Maximum encoder frequency, 39,06 KHz. */
#define ILK_DENC_SINCOS_FILTER_39_06KHZ		0x08

/** @} */

/**
 * @defgroup IL_CONST_DSIG_INJ Disturbance signal injection points
 * @{
 */

/** None. */
#define ILK_DSIG_INJ_NONE		0x00
/** Torque command source. */
#define ILK_DSIG_INJ_CSRC_TOR		0x01
/** Torque demand. */
#define ILK_DSIG_INJ_DEMAND_TOR		0x02
/** Torque control loop. */
#define ILK_DSIG_INJ_CLOOP_TOR		0x03
/** Velocity command source. */
#define ILK_DSIG_INJ_CSRC_VEL		0x04
/** Velocity demand. */
#define ILK_DSIG_INJ_DEMAND_VEL		0x05
/** Velocity control loop. */
#define ILK_DSIG_INJ_CLOOP_VEL		0x06
/** Position command source. */
#define ILK_DSIG_INJ_CSRC_POS		0x07
/** Position command source. */
#define ILK_DSIG_INJ_DEMAND_POS		0x08
/** Position control loop. */
#define ILK_DSIG_INJ_CLOOP_POS		0x09

/** @} */

/**
 * @defgroup IL_CONST_CMD_SRC Command sources
 * @{
 */

/** Network. */
#define ILK_CMD_SRC_NETWORK		0x00
/** Analog input. */
#define ILK_CMD_SRC_AIN			0x01
/** PWM or PWM & direction. */
#define ILK_CMD_SRC_PWM			0x02
/** Step & direction. */
#define ILK_CMD_SRC_STEPDIR		0x03
/** Electronic gearing. */
#define ILK_CMD_SRC_EGEARING		0x05
/** Integral generator. */
#define ILK_CMD_SRC_INTEGRAL_GEN	0x06

/** @} */

/** @} */

#endif
