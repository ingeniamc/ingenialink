#include "public/ingenialink/registers.h"


const il_reg_t IL_REG_ECAT_STORE_ALL = {
	.subnode = 1,
	.address = 0x06DB,
	.dtype = IL_REG_DTYPE_U32,
	.access = IL_REG_ACCESS_RW,
	.phy = IL_REG_PHY_NONE,
	.range = {
		.min.u32 = 0,
		.max.u32 = UINT32_MAX
	},
	.labels = NULL,
	.enums = NULL,
	.enums_count = 0
};