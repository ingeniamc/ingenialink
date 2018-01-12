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

#include <assert.h>

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

	/* parse: address */
	param = xmlGetProp(node, (const xmlChar *)"address");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing address)", id);
		return IL_EFAIL;
	}

	kh_val(dict->h, k).address = strtoul((char *)param, NULL, 16);

	xmlFree(param);

	/* parse: dtype */
	param = xmlGetProp(node, (const xmlChar *)"dtype");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing dtype)", id);
		return IL_EFAIL;
	}

	r = get_dtype((char *)param, &kh_val(dict->h, k).dtype);
	xmlFree(param);
	if (r < 0)
		return r;

	/* parse: dtype */
	param = xmlGetProp(node, (const xmlChar *)"access");
	if (!param) {
		ilerr__set("Malformed entry (%s, missing access)", id);
		return IL_EFAIL;
	}

	r = get_access((char *)param, &kh_val(dict->h, k).access);
	xmlFree(param);
	if (r < 0)
		return r;

	/* parse: phyisical units (optional) */
	param = xmlGetProp(node, (const xmlChar *)"phy");
	if (param) {
		kh_val(dict->h, k).phy = get_phy((char *)param);
		xmlFree(param);
	} else {
		kh_val(dict->h, k).phy = IL_REG_PHY_NONE;
	}

	return 0;
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
		if (kh_exist(dict->h, k))
			xmlFree((char *)kh_key(dict->h, k));
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

	assert(dict);

	for (k = 0; k < kh_end(dict->h); ++k) {
		if (kh_exist(dict->h, k))
			xmlFree((char *)kh_key(dict->h, k));
	}

	kh_destroy(str, dict->h);

	free(dict);
}

int il_dict_get(il_dict_t *dict, const char *id, il_reg_t **reg)
{
	khint_t k;

	assert(dict);
	assert(id);
	assert(reg);

	k = kh_get(str, dict->h, id);
	if (k == kh_end(dict->h)) {
		ilerr__set("Register not found");
		return IL_EFAIL;
	}

	*reg = &kh_value(dict->h, k);

	return 0;
}
