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

/** Motor parameters: pole pitch. */
extern const il_reg_t IL_REG_MOTPARAM_PPITCH;

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
