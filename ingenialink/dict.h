/*
 * MIT License
 *
 * Copyright (c) 2017-2018 Ingenia-CAT S.L.
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

#ifndef DICT_H_
#define DICT_H_

#include "public/ingenialink/dict.h"

#include "klib/khash.h"

/** khash type for reg_id<->register dictionary. */
KHASH_MAP_INIT_STR(reg_id, il_reg_t)

/** khash type for cat_id<->labels dictionary. */
KHASH_MAP_INIT_STR(cat_id, il_dict_labels_t *)

/** Dictionary root name. */
#define ROOT_NAME	"IngeniaDictionary"

/** XPath for categories. */
#define XPATH_CATS	"//Categories/Category"

/** XPath for registers. */
#define XPATH_REGS	"//Registers/Register"

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
	/** Categories hash table. */
	khash_t(cat_id) * h_cats;
	/** Registers hash table. */
	khash_t(reg_id) * h_regs;
};

#endif
