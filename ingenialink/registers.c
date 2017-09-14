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
