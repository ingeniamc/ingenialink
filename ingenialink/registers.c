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

#include "ingenialink/registers.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

/* Vendor specific (Ingenia) */
const il_reg_t IL_REG_PAIR_POLES = {
	0x2301, 0x00, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_FB_VEL_SENSOR = {
	0x2310, 0x01, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_FB_POS_SENSOR = {
	0x2310, 0x03, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_SSI_STURNBITS = {
	0x2380, 0x05, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_MOTPARAM_PPITCH = {
	0x2701, 0x03, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor config: sampling rage (period). */
const il_reg_t IL_REG_MONITOR_CFG_T_S = {
	0x2c50, 0x01, IL_REG_DTYPE_U16, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor config: enable mode. */
const il_reg_t IL_REG_MONITOR_CFG_ENABLE = {
	0x2c50, 0x02, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor config: trigger delay in samples. */
const il_reg_t IL_REG_MONITOR_CFG_DELAY_SAMPLES = {
	0x2c50, 0x03, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor result: max entry number. */
const il_reg_t IL_REG_MONITOR_RESULT_SZ = {
	0x2c51, 0x01, IL_REG_DTYPE_U16, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor result: filled entry values. */
const il_reg_t IL_REG_MONITOR_RESULT_FILLED = {
	0x2c51, 0x02, IL_REG_DTYPE_U16, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor result: filled entry values. */
const il_reg_t IL_REG_MONITOR_RESULT_ENTRY = {
	0x2c51, 0x03, IL_REG_DTYPE_U16, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor result: actual entry 1. */
const il_reg_t IL_REG_MONITOR_RESULT_CH_1 = {
	0x2c51, 0x04, IL_REG_DTYPE_S32, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor result: actual entry 2. */
const il_reg_t IL_REG_MONITOR_RESULT_CH_2 = {
	0x2c51, 0x05, IL_REG_DTYPE_S32, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor result: actual entry 3. */
const il_reg_t IL_REG_MONITOR_RESULT_CH_3 = {
	0x2c51, 0x06, IL_REG_DTYPE_S32, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor result: actual entry 4. */
const il_reg_t IL_REG_MONITOR_RESULT_CH_4 = {
	0x2c51, 0x07, IL_REG_DTYPE_S32, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

/** Monitor mapping: channel 1. */
const il_reg_t IL_REG_MONITOR_MAP_CH_1 = {
	0x2c52, 0x01, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor mapping: channel 2. */
const il_reg_t IL_REG_MONITOR_MAP_CH_2 = {
	0x2c52, 0x02, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor mapping: channel 3. */
const il_reg_t IL_REG_MONITOR_MAP_CH_3 = {
	0x2c52, 0x03, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor mapping: channel 4. */
const il_reg_t IL_REG_MONITOR_MAP_CH_4 = {
	0x2c52, 0x04, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, mode. */
const il_reg_t IL_REG_MONITOR_TRIG_MODE = {
	0x2c55, 0x01, IL_REG_DTYPE_U8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, source register. */
const il_reg_t IL_REG_MONITOR_TRIG_SRC = {
	0x2c55, 0x02, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, positive threshold. */
const il_reg_t IL_REG_MONITOR_TRIG_TH_POS = {
	0x2c55, 0x03, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, negative threshold. */
const il_reg_t IL_REG_MONITOR_TRIG_TH_NEG = {
	0x2c55, 0x04, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, digital input mask. */
const il_reg_t IL_REG_MONITOR_TRIG_DIN_MSK = {
	0x2c55, 0x05, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/** Monitor trigger, delay in samples. */
const il_reg_t IL_REG_MONITOR_TRIG_DELAY = {
	0x2c55, 0x06, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

/* CiA 402 */
const il_reg_t IL_REG_CTL_WORD = {
	0x6040, 0x00, IL_REG_DTYPE_U16, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_STS_WORD = {
	0x6041, 0x00, IL_REG_DTYPE_U16, IL_REG_ACCESS_RO, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_OP_MODE = {
	0x6060, 0x00, IL_REG_DTYPE_S8, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_POS_ACT = {
	0x6064, 0x00, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_POS
};

const il_reg_t IL_REG_VEL_ACT = {
	0x606C, 0x00, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_VEL
};

const il_reg_t IL_REG_TORQUE_TGT = {
	0x6071, 0x00, IL_REG_DTYPE_S16, IL_REG_ACCESS_RW, IL_REG_PHY_TORQUE
};

const il_reg_t IL_REG_RATED_TORQUE = {
	0x6076, 0x00, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_TORQUE_ACT = {
	0x6077, 0x00, IL_REG_DTYPE_S16, IL_REG_ACCESS_RW, IL_REG_PHY_TORQUE
};

const il_reg_t IL_REG_POS_TGT = {
	0x607A, 0x00, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_POS
};

const il_reg_t IL_REG_PRES_ENC_INCR = {
	0x608F, 0x01, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_PRES_MOTOR_REVS = {
	0x608F, 0x02, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VRES_ENC_INCR = {
	0x6090, 0x01, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VRES_MOTOR_REVS = {
	0x6090, 0x02, IL_REG_DTYPE_U32, IL_REG_ACCESS_RW, IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VEL_TGT = {
	0x60FF, 0x00, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_VEL
};
