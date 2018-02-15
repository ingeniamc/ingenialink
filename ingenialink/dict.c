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

#include "dict.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "ingenialink/err.h"
#include "ingenialink/utils.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/** Dummy libxml2 error function (so that no garbage is put to stderr/stdout) */
static void xml_error(void *ctx, const char *msg, ...)
{
	(void)ctx;
	(void)msg;
}

/**
 * Obtain data type from dictionary name.
 *
 * @param [in] name
 *	Name.
 * @param [out] dtype
 *	Where data type will be stored.
 *
 * @return
 *	0 on success, IL_EINVAL if unknown.
 */
static int get_dtype(const char *name, il_reg_dtype_t *dtype)
{
	static const il_dict_dtype_map_t map[] = {
		{ "u8", IL_REG_DTYPE_U8 },
		{ "s8", IL_REG_DTYPE_S8 },
		{ "u16", IL_REG_DTYPE_U16 },
		{ "s16", IL_REG_DTYPE_S16 },
		{ "u32", IL_REG_DTYPE_U32 },
		{ "s32", IL_REG_DTYPE_S32 },
		{ "u64", IL_REG_DTYPE_U64 },
		{ "s64", IL_REG_DTYPE_S64 },
		{ "float", IL_REG_DTYPE_FLOAT },
		{ "str", IL_REG_DTYPE_STR },
	};

	size_t i;

	for (i = 0; i < ARRAY_SIZE(map); i++) {
		if (strcmp(map[i].name, name) == 0) {
			*dtype = map[i].dtype;
			return 0;
		}
	}

	ilerr__set("Data type not supported (%s)", name);
	return IL_EINVAL;
}

/**
 * Obtain access type from dictionary name.
 *
 * @param [in] name
 *	Name.
 * @param [out] access
 *	Where access type will be stored.
 *
 * @return
 *	0 on success, IL_EINVAL if unknown.
 */
static int get_access(const char *name, il_reg_access_t *access)
{
	static const il_dict_access_map_t map[] = {
		{ "r", IL_REG_ACCESS_RO },
		{ "w", IL_REG_ACCESS_WO },
		{ "rw", IL_REG_ACCESS_RW },
	};

	size_t i;

	for (i = 0; i < ARRAY_SIZE(map); i++) {
		if (strcmp(map[i].name, name) == 0) {
			*access = map[i].access;
			return 0;
		}
	}

	ilerr__set("Access type not supported (%s)", name);
	return IL_EINVAL;
}

/**
 * Obtain physical units type from dictionary name.
 *
 * @param [in] name
 *	Name.
 *
 * @return
 *	Physical units type (defaults to IL_REG_PHY_NONE if unknown).
 */
static il_reg_phy_t get_phy(const char *name)
{
	static const il_dict_phy_map_t map[] = {
		{ "none", IL_REG_PHY_NONE },
		{ "torque", IL_REG_PHY_TORQUE },
		{ "pos", IL_REG_PHY_POS },
		{ "vel", IL_REG_PHY_VEL },
		{ "acc", IL_REG_PHY_ACC },
		{ "volt_rel", IL_REG_PHY_VOLT_REL },
		{ "rad", IL_REG_PHY_RAD },
	};

	size_t i;

	for (i = 0; i < ARRAY_SIZE(map); i++) {
		if (strcmp(map[i].name, name) == 0)
			return map[i].phy;
	}

	return IL_REG_PHY_NONE;
}

/**
 * Parse register labels.
 *
 * @param [in] node
 *	XML Node.
 * @param [in, out] reg
 *	Register.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int parse_labels(xmlNodePtr node, il_reg_t *reg)
{
	int r;
	xmlNode *label;

	reg->labels = il_reg_labels_create();
	if (!reg->labels)
		return IL_EFAIL;

	for (label = node->children; label; label = label->next) {
		xmlChar *lang, *content;

		if (label->type != XML_ELEMENT_NODE)
			continue;

		lang = xmlGetProp(label, (const xmlChar *)"lang");
		if (!lang) {
			ilerr__set("Malformed label entry");
			r = IL_EFAIL;
			goto cleanup_labels;
		}

		content = xmlNodeGetContent(label);
		if (content) {
			il_reg_labels_set(reg->labels,
					  (const char *)lang,
					  (const char *)content);
			xmlFree(content);
		}

		xmlFree(lang);
	}

	return 0;

cleanup_labels:
	il_reg_labels_destroy(reg->labels);

	return r;
}

/**
 * Parse register range.
 *
 * @param [in] node
 *	XML Node.
 * @param [in, out] reg
 *	Register.
 */
static void parse_range(xmlNodePtr node, il_reg_t *reg)
{
	xmlChar *val;

	val = xmlGetProp(node, (const xmlChar *)"min");
	if (val) {
		switch (reg->dtype) {
		case IL_REG_DTYPE_U8:
			reg->range.min.u8 = (uint8_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S8:
			reg->range.min.s8 = (int8_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U16:
			reg->range.min.u16 = (uint16_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S16:
			reg->range.min.s16 = (int16_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U32:
			reg->range.min.u32 = (uint32_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S32:
			reg->range.min.s32 = (int32_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U64:
			reg->range.min.u64 = (uint64_t)strtoull(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S64:
			reg->range.min.s64 = (int64_t)strtoll(
				(const char *)val, NULL, 0);
			break;
		default:
			break;
		}

		xmlFree(val);
	}

	val = xmlGetProp(node, (const xmlChar *)"max");
	if (val) {
		switch (reg->dtype) {
		case IL_REG_DTYPE_U8:
			reg->range.max.u8 = (uint8_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S8:
			reg->range.max.s8 = (int8_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U16:
			reg->range.max.u16 = (uint16_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S16:
			reg->range.max.s16 = (int16_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U32:
			reg->range.max.u32 = (uint32_t)strtoul(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S32:
			reg->range.max.s32 = (int32_t)strtol(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_U64:
			reg->range.max.u64 = (uint64_t)strtoull(
				(const char *)val, NULL, 0);
			break;
		case IL_REG_DTYPE_S64:
			reg->range.max.s64 = (int64_t)strtoll(
				(const char *)val, NULL, 0);
			break;
		default:
			break;
		}

		xmlFree(val);
	}
}

/**
 * Parse register properties.
 *
 * @param [in] node
 *	XML Node.
 * @param [in, out] reg
 *	Register.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int parse_props(xmlNodePtr node, il_reg_t *reg)
{
	int r;
	xmlNode *prop;

	for (prop = node->children; prop; prop = prop->next) {
		if (prop->type != XML_ELEMENT_NODE)
			continue;

		if (xmlStrcmp(prop->name, (const xmlChar *)"Labels") == 0) {
			r = parse_labels(prop, reg);
			if (r < 0)
				return r;
		}

		if (xmlStrcmp(prop->name, (const xmlChar *)"Range") == 0)
			parse_range(prop, reg);
	}

	return 0;
}

/**
 * Parse register node.
 *
 * @param [in] node
 *	XML Node.
 * @param [in, out] dict
 *	Dictionary instance.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int parse_register(xmlNodePtr node, il_dict_t *dict)
{
	int r, absent;
	khint_t k;
	il_reg_t *reg;
	xmlChar *id, *param;

	/* parse: id (required), insert to hash table */
	id = xmlGetProp(node, (const xmlChar *)"id");
	if (!id) {
		ilerr__set("Malformed entry (id missing)");
		return IL_EFAIL;
	}

	k = kh_put(str, dict->h, (char *)id, &absent);
	if (!absent) {
		ilerr__set("Found duplicated register: %s", id);
		xmlFree(id);
		return IL_EFAIL;
	}

	reg = &kh_val(dict->h, k);

	/* parse: address */
	param = xmlGetProp(node, (const xmlChar *)"address");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing address)", id);
		return IL_EFAIL;
	}

	reg->address = strtoul((char *)param, NULL, 16);

	xmlFree(param);

	/* parse: dtype */
	param = xmlGetProp(node, (const xmlChar *)"dtype");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing dtype)", id);
		return IL_EFAIL;
	}

	r = get_dtype((char *)param, &reg->dtype);
	xmlFree(param);
	if (r < 0)
		return r;

	/* parse: dtype */
	param = xmlGetProp(node, (const xmlChar *)"access");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing access)", id);
		return IL_EFAIL;
	}

	r = get_access((char *)param, &reg->access);
	xmlFree(param);
	if (r < 0)
		return r;

	/* parse: phyisical units (optional) */
	param = xmlGetProp(node, (const xmlChar *)"phy");
	if (param) {
		reg->phy = get_phy((char *)param);
		xmlFree(param);
	} else {
		reg->phy = IL_REG_PHY_NONE;
	}

	/* assign default min/max */
	switch (reg->dtype) {
	case IL_REG_DTYPE_U8:
		reg->range.min.u8 = 0;
		reg->range.max.u8 = UINT8_MAX;
		break;
	case IL_REG_DTYPE_S8:
		reg->range.min.s8 = INT8_MIN;
		reg->range.max.s8 = INT8_MAX;
		break;
	case IL_REG_DTYPE_U16:
		reg->range.min.u16 = 0;
		reg->range.max.u16 = UINT16_MAX;
		break;
	case IL_REG_DTYPE_S16:
		reg->range.min.s16 = INT16_MIN;
		reg->range.max.s16 = INT16_MAX;
		break;
	case IL_REG_DTYPE_U32:
		reg->range.min.u32 = 0;
		reg->range.max.u32 = UINT32_MAX;
		break;
	case IL_REG_DTYPE_S32:
		reg->range.min.s32 = INT32_MIN;
		reg->range.max.s32 = INT32_MAX;
		break;
	case IL_REG_DTYPE_U64:
		reg->range.min.u64 = 0;
		reg->range.max.u64 = UINT64_MAX;
		break;
	case IL_REG_DTYPE_S64:
		reg->range.min.s64 = INT64_MIN;
		reg->range.max.s64 = INT64_MAX;
		break;
	default:
		break;
	}

	/* parse: nested properties (e.g. labels, ranges, etc.) */
	return parse_props(node, reg);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_dict_t *il_dict_create(const char *dict_f)
{
	int r = 0, i;

	il_dict_t *dict;

	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;
	xmlXPathContextPtr xpath;
	xmlXPathObjectPtr obj;
	xmlNodePtr root;

	khint_t k;

	dict = malloc(sizeof(*dict));
	if (!dict) {
		ilerr__set("Dictionary allocation failed");
		return NULL;
	}

	/* create hash table for registers */
	dict->h = kh_init(str);
	if (!dict->h) {
		ilerr__set("Dictionary hash table allocation failed");
		r = IL_EFAIL;
		goto cleanup_dict;
	}

	/* set library error function (to prevent stdout/stderr garbage) */
	xmlSetGenericErrorFunc(NULL, xml_error);

	/* initialize parser context and parse dictionary */
	ctxt = xmlNewParserCtxt();
	if (!ctxt) {
		ilerr__set("XML context allocation failed");
		r = IL_EFAIL;
		goto cleanup_dict;
	}

	doc = xmlCtxtReadFile(ctxt, dict_f, NULL, 0);
	if (!doc) {
		ilerr__set("xml: %s", xmlCtxtGetLastError(ctxt)->message);
		r = IL_EFAIL;
		goto cleanup_ctxt;
	}

	/* verify root */
	root = xmlDocGetRootElement(doc);
	if (xmlStrcmp(root->name, (const xmlChar *)ROOT_NAME) != 0) {
		ilerr__set("Unsupported dictionary format");
		r = IL_EFAIL;
		goto cleanup_doc;
	}

	/* create and evaluate XPath to find all registers */
	xpath = xmlXPathNewContext(doc);
	if (!xpath) {
		ilerr__set("xml: %s", xmlCtxtGetLastError(ctxt)->message);
		r = IL_EFAIL;
		goto cleanup_doc;
	}

	obj = xmlXPathEvalExpression((const xmlChar *)XPATH_REGS, xpath);
	if (!obj) {
		ilerr__set("xml: %s", xmlCtxtGetLastError(ctxt)->message);
		r = IL_EFAIL;
		goto cleanup_xpath;
	}

	/* parse each register */
	for (i = 0; i < obj->nodesetval->nodeNr; i++) {
		xmlNodePtr node = obj->nodesetval->nodeTab[i];

		r = parse_register(node, dict);
		if (r < 0)
			goto cleanup_h;
	}

	goto cleanup_obj;

cleanup_h:
	for (k = 0; k < kh_end(dict->h); ++k) {
		if (kh_exist(dict->h, k)) {
			il_reg_t *reg;

			reg = &kh_value(dict->h, k);
			if (reg->labels)
				il_reg_labels_destroy(reg->labels);

			xmlFree((char *)kh_key(dict->h, k));
		}
	}

cleanup_obj:
	xmlXPathFreeObject(obj);

cleanup_xpath:
	xmlXPathFreeContext(xpath);

cleanup_doc:
	xmlFreeDoc(doc);

cleanup_ctxt:
	xmlFreeParserCtxt(ctxt);

cleanup_dict:
	if (r < 0) {
		free(dict);
		return NULL;
	}

	return dict;
}

void il_dict_destroy(il_dict_t *dict)
{
	khint_t k;

	for (k = 0; k < kh_end(dict->h); ++k) {
		if (kh_exist(dict->h, k)) {
			il_reg_t *reg;

			reg = &kh_value(dict->h, k);
			if (reg->labels)
				il_reg_labels_destroy(reg->labels);

			xmlFree((char *)kh_key(dict->h, k));
		}
	}

	kh_destroy(str, dict->h);

	free(dict);
}

int il_dict_reg_get(il_dict_t *dict, const char *id, const il_reg_t **reg)
{
	khint_t k;

	k = kh_get(str, dict->h, id);
	if (k == kh_end(dict->h)) {
		ilerr__set("Register not found (%s)", id);
		return IL_EFAIL;
	}

	*reg = (const il_reg_t *)&kh_value(dict->h, k);

	return 0;
}

size_t il_dict_nregs_get(il_dict_t *dict)
{
	return (size_t)kh_size(dict->h);
}

const char **il_dict_ids_get(il_dict_t *dict)
{
	const char **ids;
	size_t i;
	khint_t k;

	/* allocate array for register keys */
	ids = malloc(sizeof(const char *) * (il_dict_nregs_get(dict) + 1));
	if (!ids) {
		ilerr__set("Registers array allocation failed");
		return NULL;
	}

	/* assign keys, null-terminate */
	for (i = 0, k = 0; k < kh_end(dict->h); ++k) {
		if (kh_exist(dict->h, k)) {
			ids[i] = (const char *)kh_key(dict->h, k);
			i++;
		}
	}

	ids[i] = NULL;

	return ids;
}

void il_dict_ids_destroy(const char **ids)
{
	free((char **)ids);
}
