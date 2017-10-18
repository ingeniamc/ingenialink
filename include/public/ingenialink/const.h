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
 * @defgroup IL_CONST_MOTOR Motor types
 * @{
 */

/** Squirrel cage induction. */
#define IL_MOTOR_SQUIRREL	0x0007
/** Stepper with microstepping capability. */
#define IL_MOTOR_STEPPER	0x0009
/** Rotary brushless AC. */
#define IL_MOTOR_ROT_BLAC	0x000A
/** Rotary brushless DC. */
#define IL_MOTOR_ROT_BLDC	0x000B
/** Rotary DC. */
#define IL_MOTOR_ROT_DC		0x000D
/** Rotary voice coil. */
#define IL_MOTOR_ROT_VC		0x8000
/** Linear brushless AC. */
#define IL_MOTOR_LIN_BLAC	0x8001
/** Linear brushless DC. */
#define IL_MOTOR_LIN_BLDC	0x8002
/** Linear voice coil. */
#define IL_MOTOR_LIN_VC		0x8003
/** Linear DC */
#define IL_MOTOR_LIN_DC		0x8004

/** @} */

/**
 * @defgroup IL_CONST_FB Feedbacks
 * @{
 */

/** Digital encoder. */
#define IL_FB_DIGITAL_ENCODER	0x00
/** Digital halls. */
#define IL_FB_DIGITAL_HALLS	0x01
/** Analog halls. */
#define IL_FB_ANALOG_HALLS	0x02
/** Analog input. */
#define IL_FB_ANALOG_INPUT	0x04
/** SSI. */
#define IL_FB_SSI		0x05
/** SinCos. */
#define IL_FB_SINCOS		0x06
/** PWM. */
#define IL_FB_PWM		0x07
/** Resolver. */
#define IL_FB_RESOLVER		0x08
/** None. */
#define IL_FB_NONE		0x09
/** Simulated. */
#define IL_FB_SIMULATED		0x0B

/** @} */

/**
 * @defgroup IL_CONST_VEL_SENSOR Velocity sensors
 * @{
 */

/** Position sensor. */
#define IL_VEL_SENSOR_POS		0x00
/** External DC tachometer. */
#define IL_VEL_SENSOR_TACHOMETER	0x01
/** None. */
#define IL_VEL_SENSOR_NONE		0x02
/** Digital halls. */
#define IL_VEL_SENSOR_DIGITAL_HALLS	0x04

/** @} */

/**
 * @defgroup IL_CONST_POS_SENSOR Position sensors
 * @{
 */

/** Digital encoder. */
#define IL_POS_SENSOR_DIGITAL_ENCODER	0x00
/** Digital halls. */
#define IL_POS_SENSOR_DIGITAL_HALLS	0x01
/** Analog halls. */
#define IL_POS_SENSOR_ANALOG_HALLS	0x02
/** Analog input. */
#define IL_POS_SENSOR_ANALOG_INPUT	0x04
/** SSI. */
#define IL_POS_SENSOR_SSI		0x05
/** SinCos. */
#define IL_POS_SENSOR_SINCOS		0x06
/** PWM. */
#define IL_POS_SENSOR_PWM		0x07
/** Resolver. */
#define IL_POS_SENSOR_RESOLVER		0x08
/** None. */
#define IL_POS_SENSOR_NONE		0x09
/** Simulated. */
#define IL_POS_SENSOR_SIMULATED		0x0B
/** Digital tachometer. */
#define IL_POS_SENSOR_TACHOMETER	0x0C

/** @} */

/**
 * @defgroup IL_CONST_IANGLE_METHOD Initial angle determination methods
 * @{
 */

/** Forced alignment method. */
#define IL_IANGLE_METHOD_FORCED_ALIGN	0x00
/** Non incremental sensor used. */
#define IL_IANGLE_METHOD_NINCR_SENSOR	0x01
/** Initial rotor position known. */
#define IL_IANGLE_METHOD_POS_KNOWN	0x02

/** @} */

/**
 * @defgroup IL_CONST_DENC_SINCOS_PARAM Digital encoder / SinCos parameters
 * @{
 */

/** Not swapped. */
#define IL_DENC_SINCOS_NSWAPPED		0x00
/** Swapped. */
#define IL_DENC_SINCOS_SWAPPED		0x01

/** No encoder. */
#define IL_DENC_SINCOS_TYPE_NONE	0x00
/** 2 channels encoder (single ended). */
#define IL_DENC_SINCOS_TYPE_2CH_S	0x01
/** 2 channels + index encoder (single ended). */
#define IL_DENC_SINCOS_TYPE_2CH_S_IDX	0x02
/** 2 channels encoder (differential). */
#define IL_DENC_SINCOS_TYPE_2CH_D	0x03
/** 2 channels + index encoder (differential). */
#define IL_DENC_SINCOS_TYPE_2CH_D_IDX	0x04

/** Maximum encoder frequency, 30 MHz. */
#define IL_DENC_SINCOS_FILTER_30MHZ	0x00
/** Maximum encoder frequency, 10 MHz. */
#define IL_DENC_SINCOS_FILTER_10MHZ	0x01
/** Maximum encoder frequency, 5 MHz. */
#define IL_DENC_SINCOS_FILTER_5MHZ	0x02
/** Maximum encoder frequency, 2 MHz. */
#define IL_DENC_SINCOS_FILTER_2MHZ	0x03
/** Maximum encoder frequency, 1 MHz. */
#define IL_DENC_SINCOS_FILTER_1MHZ	0x04
/** Maximum encoder frequency, 625 KHz. */
#define IL_DENC_SINCOS_FILTER_625KHZ	0x05
/** Maximum encoder frequency, 312,5 KHz. */
#define IL_DENC_SINCOS_FILTER_312_5KHZ	0x06
/** Maximum encoder frequency, 156,25 KHz. */
#define IL_DENC_SINCOS_FILTER_156_25KHZ	0x07
/** Maximum encoder frequency, 39,06 KHz. */
#define IL_DENC_SINCOS_FILTER_39_06KHZ	0x08

/** @} */

/** @} */

#endif
