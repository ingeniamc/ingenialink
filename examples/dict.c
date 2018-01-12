/**
 * @example dict.c
 *
 * This example shows how to use a dictionary.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

static void print_reg(il_reg_t *reg)
{
	const char *name;

	/* address */
	printf("Address: %08x\n", reg->address);

	/* data type */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		name = "8-bit unsigned integer";
		break;
	case IL_REG_DTYPE_S8:
		name = "8-bit integer";
		break;
	case IL_REG_DTYPE_U16:
		name = "16-bit unsigned integer";
		break;
	case IL_REG_DTYPE_S16:
		name = "16-bit integer";
		break;
	case IL_REG_DTYPE_U32:
		name = "32-bit unsigned integer";
		break;
	case IL_REG_DTYPE_S32:
		name = "32-bit integer";
		break;
	case IL_REG_DTYPE_U64:
		name = "64-bit unsigned integer";
		break;
	case IL_REG_DTYPE_S64:
		name = "64-bit integer";
		break;
	default:
		name = "unknown";
	}

	printf("Data type: %s\n", name);

	/* access */
	switch (reg->access) {
	case IL_REG_ACCESS_RO:
		name = "read-only";
		break;
	case IL_REG_ACCESS_WO:
		name = "write-only";
		break;
	case IL_REG_ACCESS_RW:
		name = "read/write";
		break;
	default:
		name = "unknown";
	}

	printf("Access: %s\n", name);

	/* physical units */
	switch (reg->phy) {
	case IL_REG_PHY_NONE:
		name = "none";
		break;
	case IL_REG_PHY_TORQUE:
		name = "torque";
		break;
	case IL_REG_PHY_POS:
		name = "position";
		break;
	case IL_REG_PHY_VEL:
		name = "velocity";
		break;
	case IL_REG_PHY_ACC:
		name = "acceleration";
		break;
	case IL_REG_PHY_VOLT_REL:
		name = "relative voltage";
		break;
	case IL_REG_PHY_RAD:
		name = "radians";
		break;
	default:
		name = "unknown";
	}

	printf("Physical units: %s\n", name);
}

int main(int argc, const char **argv)
{
	int r = 0;
	il_dict_t *dict;
	il_reg_t *reg;

	if (argc < 3) {
		fprintf(stderr, "Usage: ./dict DICTIONARY.xdd REGISTER\n");
		return -1;
	}

	dict = il_dict_create(argv[1]);
	if (!dict) {
		fprintf(stderr, "Could not create dictionary: %s\n",
			ilerr_last());
		return -1;
	}

	r = il_dict_get(dict, argv[2], &reg);
	if (r < 0) {
		fprintf(stderr, "Could not find register: %s\n", ilerr_last());
		r = -1;
		goto cleanup;
	}

	print_reg(reg);

cleanup:
	il_dict_destroy(dict);

	return r;
}
