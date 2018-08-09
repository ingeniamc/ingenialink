
#include "public/ingenialink/registers.h"

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