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
const il_reg_t IL_REG_MOTPARAM_PPITCH = {
	.idx = 0x2701, .sidx = 0x03, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

/* CiA 402 */
const il_reg_t IL_REG_CTL_WORD = {
	.idx = 0x6040, .sidx = 0x00, .dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_STS_WORD = {
	.idx = 0x6041, .sidx = 0x00, .dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RO, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_OP_MODE = {
	.idx = 0x6060, .sidx = 0x00, .dtype = IL_REG_DTYPE_S8,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_POS_ACT = {
	.idx = 0x6064, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_POS
};

const il_reg_t IL_REG_VEL_ACT = {
	.idx = 0x606C, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_VEL
};

const il_reg_t IL_REG_TORQUE_TGT = {
	.idx = 0x6071, .sidx = 0x00, .dtype = IL_REG_DTYPE_S16,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_TORQUE
};

const il_reg_t IL_REG_RATED_TORQUE = {
	.idx = 0x6076, .sidx = 0x00, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_TORQUE_ACT = {
	.idx = 0x6077, .sidx = 0x00, .dtype = IL_REG_DTYPE_S16,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_TORQUE
};

const il_reg_t IL_REG_POS_TGT = {
	.idx = 0x607A, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_POS
};

const il_reg_t IL_REG_PRES_ENC_INCR = {
	.idx = 0x608F, .sidx = 0x01, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_PRES_MOTOR_REVS = {
	.idx = 0x608F, .sidx = 0x02, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VRES_ENC_INCR = {
	.idx = 0x6090, .sidx = 0x01, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VRES_MOTOR_REVS = {
	.idx = 0x6090, .sidx = 0x02, .dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_NONE
};

const il_reg_t IL_REG_VEL_TGT = {
	.idx = 0x60FF, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_VEL
};
