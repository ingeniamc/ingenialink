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

#ifndef INGENIALINK_REGISTERS_H_
#define INGENIALINK_REGISTERS_H_

#include "public/ingenialink/registers.h"

/*
 * Vendor specific (Ingenia)
 */

/** Motor pair poles. */
extern const il_reg_t IL_REG_PAIR_POLES;

/** Feedbacks: velocity sensor. */
extern const il_reg_t IL_REG_FB_VEL_SENSOR;

/** Feedbacks: position sensor. */
extern const il_reg_t IL_REG_FB_POS_SENSOR;

/** SSI: single-turn bits. */
extern const il_reg_t IL_REG_SSI_STURNBITS;

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

/** Target position. */
extern const il_reg_t IL_REG_POS_TGT;

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

#endif
