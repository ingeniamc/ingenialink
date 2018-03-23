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

#include "public/ingenialink/registers.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

/*
 * CiA 301.
 */

const il_reg_t IL_REG_SW_VERSION = {
	.address = 0x00100A,
	.dtype = IL_REG_DTYPE_U64,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u64 = 0,
		.max.u64 = UINT64_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_STORE_ALL = {
	.address = 0x011010,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_WO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_STORE_COMM = {
	.address = 0x021010,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_WO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_STORE_APP = {
	.address = 0x031010,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_WO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_ID_PROD_CODE = {
	.address = 0x021018,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_ID_REVISION = {
	.address = 0x031018,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_ID_SERIAL = {
	.address = 0x041018,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

/*
 * Vendor specific (Ingenia).
 */

const il_reg_t IL_REG_PAIR_POLES = {
	.address = 0x002301,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_FB_VEL_SENSOR = {
	.address = 0x022310,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_FB_POS_SENSOR = {
	.address = 0x032310,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_SSI_STURNBITS = {
	.address = 0x052380,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MOTPARAM_PPITCH = {
	.address = 0x032701,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MOTPARAM_STROKE = {
	.address = 0x052701,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_CFG_T_S = {
	.address = 0x012c50,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_CFG_ENABLE = {
	.address = 0x022c50,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_CFG_DELAY_SAMPLES = {
	.address = 0x032c50,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_SZ = {
	.address = 0x012c51,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_FILLED = {
	.address = 0x022c51,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_ENTRY = {
	.address = 0x032c51,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_CH_1 = {
	.address = 0x042c51,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_CH_2 = {
	.address = 0x052c51,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_CH_3 = {
	.address = 0x062c51,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_RESULT_CH_4 = {
	.address = 0x072c51,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_MAP_CH_1 = {
	.address = 0x012c52,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_MAP_CH_2 = {
	.address = 0x022c52,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_MAP_CH_3 = {
	.address = 0x032c52,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_MAP_CH_4 = {
	.address = 0x042c52,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_MODE = {
	.address = 0x012c55,
	.dtype = IL_REG_DTYPE_U8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u8 = 0,
		.max.u8 = UINT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_SRC = {
	.address = 0x022c55,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_TH_POS = {
	.address = 0x032c55,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_TH_NEG = {
	.address = 0x042c55,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_DIN_MSK = {
	.address = 0x052c55,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MONITOR_TRIG_DELAY = {
	.address = 0x062c55,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_OL_VOLTAGE = {
	.address = 0x012d00,
	.dtype = IL_REG_DTYPE_S16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_VOLT_REL,
	.range = {
		.min.s16 = INT16_MIN,
		.max.s16 = INT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_OL_FREQUENCY = {
	.address = 0x022d00,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_HW_VARIANT = {
	.address = 0x022ff4,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_DRIVE_NAME = {
	.address = 0x002ffe,
	.dtype = IL_REG_DTYPE_U64,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u64 = 0,
		.max.u64 = UINT64_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_RESET_DEVICE = {
	.address = 0x002fff,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

/* CiA 402 */
const il_reg_t IL_REG_CTL_WORD = {
	.address = 0x006040,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_STS_WORD = {
	.address = 0x006041,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_MOTOR_TYPE = {
	.address = 0x006402,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_OP_MODE = {
	.address = 0x006060,
	.dtype = IL_REG_DTYPE_S8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s8 = INT8_MIN,
		.max.s8 = INT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_OP_MODE_DISP = {
	.address = 0x006061,
	.dtype = IL_REG_DTYPE_S8,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.s8 = INT8_MIN,
		.max.s8 = INT8_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_POS_ACT = {
	.address = 0x006064,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_POS,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_VEL_ACT = {
	.address = 0x00606C,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_VEL,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_TORQUE_TGT = {
	.address = 0x006071,
	.dtype = IL_REG_DTYPE_S16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_TORQUE,
	.range = {
		.min.s16 = INT16_MIN,
		.max.s16 = INT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_RATED_TORQUE = {
	.address = 0x006076,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_TORQUE_ACT = {
	.address = 0x006077,
	.dtype = IL_REG_DTYPE_S16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_TORQUE,
	.range = {
		.min.s16 = INT16_MIN,
		.max.s16 = INT16_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_POS_TGT = {
	.address = 0x00607A,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_POS,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_PRES_ENC_INCR = {
	.address = 0x01608F,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_PRES_MOTOR_REVS = {
	.address = 0x02608F,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_VRES_ENC_INCR = {
	.address = 0x016090,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_VRES_MOTOR_REVS = {
	.address = 0x026090,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL
};

const il_reg_t IL_REG_VEL_TGT = {
	.address = 0x0060FF,
	.dtype = IL_REG_DTYPE_S32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_VEL,
	.range = {
		.min.s32 = INT32_MIN,
		.max.s32 = INT32_MAX
	},
	.labels = NULL
};
