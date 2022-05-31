/**
 * @example dict.c
 *
 * This example shows how to use a dictionary.
 */

#include <stdio.h>
#include <inttypes.h>
#include <ingenialink/ingenialink.h>
#include "ingenialink/log.h"

static void print_scat(const char *id, il_dict_labels_t *labels)
{
	/* id */
	log_info("\tID: %s", id);

	/* labels */
	log_info("\tLabels:");

	if (labels && il_dict_labels_nlabels_get(labels) > 0) {
		size_t i;
		const char **langs;

		langs = il_dict_labels_langs_get(labels);

		for (i = 0; langs[i]; i++) {
			const char *label;

			(void)il_dict_labels_get(labels, langs[i], &label);

			log_info("\t\t%s: %s", langs[i], label);
		}

		il_dict_labels_langs_destroy(langs);
	} else {
		log_info("\t\tNone");
	}
}

static void print_cat(il_dict_t *dict, const char *id, il_dict_labels_t *labels)
{
	size_t i;
	const char **ids;

	/* id */
	log_info("ID: %s", id);

	/* labels */
	log_info("Labels:");

	if (labels && il_dict_labels_nlabels_get(labels) > 0) {
		size_t i;
		const char **langs;

		langs = il_dict_labels_langs_get(labels);

		for (i = 0; langs[i]; i++) {
			const char *label;

			(void)il_dict_labels_get(labels, langs[i], &label);

			log_info("\t%s: %s", langs[i], label);
		}

		il_dict_labels_langs_destroy(langs);
	} else {
		log_info("\tNone");
	}

	/* subcategories */
	log_info("Sub-categories:");
	ids = il_dict_scat_ids_get(dict, id);
	if (!ids) {
		log_error("Could not obtain sub-categories: %s", ilerr_last());
		return;
	}

	for (i = 0; ids[i]; i++) {
		il_dict_labels_t *labels;

		(void)il_dict_scat_get(dict, id, ids[i], &labels);
		print_scat(ids[i], labels);
	}

	il_dict_scat_ids_destroy(ids);

	log_info("==============================");
}

static void print_reg(const il_reg_t *reg)
{
	const char *name;
	const char **langs;
	size_t i;

	/* address */
	log_info("Address: %08x", reg->address);

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

	log_info("Data type: %s", name);

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

	log_info("Access: %s", name);

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

	/* physical units */
	log_info("Physical units: %s", name);

	/* range */
	log_info("Range: ");

	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		log_info("(%"PRIu8", %"PRIu8")",
		       reg->range.min.u8, reg->range.max.u8);
		break;
	case IL_REG_DTYPE_S8:
		log_info("(%"PRId8", %"PRId8")",
		       reg->range.min.s8, reg->range.max.s8);
		break;
	case IL_REG_DTYPE_U16:
		log_info("(%"PRIu16", %"PRIu16")",
		       reg->range.min.u16, reg->range.max.u16);
		break;
	case IL_REG_DTYPE_S16:
		log_info("(%"PRId16", %"PRId16")",
		       reg->range.min.s16, reg->range.max.s16);
		break;
	case IL_REG_DTYPE_U32:
		log_info("(%"PRIu32", %"PRIu32")",
		       reg->range.min.u32, reg->range.max.u32);
		break;
	case IL_REG_DTYPE_S32:
		log_info("(%"PRId32", %"PRId32")",
		       reg->range.min.s32, reg->range.max.s32);
		break;
	case IL_REG_DTYPE_U64:
		log_info("(%"PRIu64", %"PRIu64")",
		       reg->range.min.u64, reg->range.max.u64);
		break;
	case IL_REG_DTYPE_S64:
		log_info("(%"PRId64", %"PRId64")",
		       reg->range.min.s64, reg->range.max.s64);
		break;
	default:
		log_info("Undefined");
	}

	/* labels */
	log_info("Labels:");

	if (reg->labels && il_dict_labels_nlabels_get(reg->labels) > 0) {
		langs = il_dict_labels_langs_get(reg->labels);

		for (i = 0; langs[i]; i++) {
			const char *label;

			(void)il_dict_labels_get(reg->labels, langs[i], &label);

			log_info("\t%s: %s", langs[i], label);
		}

		il_dict_labels_langs_destroy(langs);
	} else {
		log_info("\tNone");
	}

	/* category and subcategory */
	log_info("Category ID: %s", reg->cat_id);
	log_info("Sub-category ID: %s", reg->scat_id);

	log_info("==============================");
}

int main(int argc, const char **argv)
{
	int r = 0;
	il_dict_t *dict;
	const char **ids;
	size_t i;
	const il_reg_t *reg;

	if (argc < 2) {
		log_error("Usage: ./dict DICTIONARY.xml");
		return -1;
	}

	dict = il_dict_create(argv[1]);
	if (!dict) {
		log_error("Could not create dictionary: %s",
			ilerr_last());
		return -1;
	}

	/* show categories */
	ids = il_dict_cat_ids_get(dict);
	if (!ids) {
		log_error("Could not obtain categories: %s", ilerr_last());
		goto cleanup;
	}

	for (i = 0; ids[i]; i++) {
		il_dict_labels_t *labels;

		(void)il_dict_cat_get(dict, ids[i], &labels);
		print_cat(dict, ids[i], labels);
	}

	il_dict_cat_ids_destroy(ids);

	/* show registers */
	ids = il_dict_reg_ids_get(dict);
	if (!ids) {
		log_error("Could not obtain IDs: %s", ilerr_last());
		goto cleanup;
	}

	for (i = 0; ids[i]; i++) {
		(void)il_dict_reg_get(dict, ids[i], &reg);
		print_reg(reg);
	}

	il_dict_reg_ids_destroy(ids);

cleanup:
	il_dict_destroy(dict);

	return r;
}
