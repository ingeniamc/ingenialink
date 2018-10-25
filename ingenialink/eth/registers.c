
#include "public/ingenialink/registers.h"

const il_reg_t IL_REG_MCB_CTL_WORD = {
	.subnode = 1,
	.address = 0x0010,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL,
	.enums = NULL,
	.enums_count = 0
};

const il_reg_t IL_REG_MCB_STS_WORD = {
	.subnode = 1,
	.address = 0x0011,
	.dtype = IL_REG_DTYPE_U16,
	.access = IL_REG_ACCESS_RO,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u16 = 0,
		.max.u16 = UINT16_MAX
	},
	.labels = NULL,
	.enums = NULL,
	.enums_count = 0
};