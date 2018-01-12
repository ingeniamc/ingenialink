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

#ifndef PUBLIC_INGENIALINK_DICT_H_
#define PUBLIC_INGENIALINK_DICT_H_

#include "common.h"

#include "registers.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/dict.h
 * @brief Dictionary
 * @defgroup IL_DICT Dictionary
 * @ingroup IL
 * @{
 */

/** IngeniaLink dictionary. */
typedef struct il_dict il_dict_t;

/**
 * Create a dictionary.
 *
 * @params [in] dict_f
 *	Dictionary file.
 *
 * @return
 *	  Dictionary instance.
 */
IL_EXPORT il_dict_t *il_dict_create(const char *dict_f);

/**
 * Destroy a dictionary.
 *
 * @params [in, out] dict
 *	Dictionary instance.
 */
IL_EXPORT void il_dict_destroy(il_dict_t *dict);

/**
 * Obtain register from ID.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] id
 *	Register ID.
 * @param [out] reg
 *	Where register with given ID will be stored.
 *
 * @return
 *	0 on success, IL_EFAIL if the register does not exist.
 */
IL_EXPORT int il_dict_get(il_dict_t *dict, const char *id, il_reg_t **reg);

/** @} */

IL_END_DECL

#endif

