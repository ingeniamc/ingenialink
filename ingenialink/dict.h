#ifndef DICT_H_
#define DICT_H_

#include "public/ingenialink/dict.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "klib/khash.h"

/** Number string length (enough to fit all numbers). */
#define NUM_STR_LEN	25
/** Number of subnodes by default. */
#define INITIAL_SUBNODES 4

/** Register container. */
typedef struct {
	/** Register. */
	il_reg_t reg;
	/** XML node. */
	xmlNodePtr xml_node;
} il_dict_reg_t;

/** khash type for reg_id<->register dictionary. */
KHASH_MAP_INIT_STR(reg_id, il_dict_reg_t)

/** khash type for scat_id<->labels dictionary. */
KHASH_MAP_INIT_STR(scat_id, il_dict_labels_t *)

/** Category container. */
typedef struct {
	/** Labels. */
	il_dict_labels_t *labels;
	/** Sub-categories hash table. */
	khash_t(scat_id) * h_scats;
} il_dict_cat_t;

/** khash type for cat_id<->labels dictionary. */
KHASH_MAP_INIT_STR(cat_id, il_dict_cat_t)

/** Dictionary root name. */
#define ROOT_NAME	"IngeniaDictionary"

/** XPath for categories. */
#define XPATH_CATS	"//Categories/Category"

/** XPath for registers. */
#define XPATH_REGS	"//Registers/Register"

/** XPath for axis. */
#define XPATH_AXES	"//Axes/Axis"

/** XPath for version. */
#define XPATH_VERSION	"//Header/Version"

/** Data type mapping. */
typedef struct {
	/** Name. */
	const char *name;
	/** Data type. */
	il_reg_dtype_t dtype;
} il_dict_dtype_map_t;

/** Access type mapping. */
typedef struct {
	/** Name. */
	const char *name;
	/** Access type. */
	il_reg_access_t access;
} il_dict_access_map_t;

/** Physical units type mapping. */
typedef struct {
	/** Name. */
	const char *name;
	/** Access type. */
	il_reg_phy_t phy;
} il_dict_phy_map_t;

/** IngeniaLink dictionary. */
struct il_dict {
	/** XML parser context. */
	xmlParserCtxtPtr xml_ctxt;
	/** XML document. */
	xmlDocPtr xml_doc;
	/** Categories hash table. */
	khash_t(cat_id) *h_cats;
	/** Registers hash table. */
	khash_t(reg_id) **h_regs;
	/** Dictionary version. */
	const char *version;
	/** Dictionary subnodes. */
	int subnodes;
};

#endif
