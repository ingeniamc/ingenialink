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

#include "dict_labels.h"
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
 * @param [in] dict_f
 *	Dictionary file.
 *
 * @return
 *	  Dictionary instance.
 */
IL_EXPORT il_dict_t *il_dict_create(const char *dict_f);

/**
 * Destroy a dictionary.
 *
 * @param [in, out] dict
 *	Dictionary instance.
 */
IL_EXPORT void il_dict_destroy(il_dict_t *dict);

/**
 * Save a dictionary.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] fname
 *	Output file name/path.
 *
 * @return
 *	0 on success, IL_EFAIL if the dictionary could not be saved.
 */
IL_EXPORT int il_dict_save(il_dict_t *dict, const char *fname);

/**
 * Obtain category labels from ID.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] cat_id
 *	Category ID.
 * @param [out] labels
 *	Where labels for the given ID will be stored.
 *
 * @return
 *	0 on success, IL_EFAIL if the register does not exist.
 */
IL_EXPORT int il_dict_cat_get(il_dict_t *dict, const char *cat_id,
			      il_dict_labels_t **labels);

/**
 * Obtain number of categories in the dictionary.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Number of categories in the dictionary.
 */
IL_EXPORT size_t il_dict_cat_cnt(il_dict_t *dict);

/**
 * Obtain the list of category IDs.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Category IDs (NULL if none or error).
 *
 * @see
 *	il_dict_cat_ids_destroy
 */
IL_EXPORT const char **il_dict_cat_ids_get(il_dict_t *dict);

/**
 * Destroy the list of obtained category IDs.
 *
 * @param [in] cat_ids
 *	Categories IDs.
 *
 * @see
 *	il_dict_cat_ids_get
 */
IL_EXPORT void il_dict_cat_ids_destroy(const char **cat_ids);

/**
 * Obtain sub-category labels from a category.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] cat_id
 *	Category ID.
 * @param [in] scat_id
 *	Sub-category ID.
 * @param [out] labels
 *	Where labels for the given ID will be stored.
 *
 * @return
 *	0 on success, IL_EFAIL if the register does not exist.
 */
IL_EXPORT int il_dict_scat_get(il_dict_t *dict, const char *cat_id,
			       const char *scat_id, il_dict_labels_t **labels);

/**
 * Obtain number of sub-categories in a category.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] cat_id
 *	Category ID.
 *
 * @return
 *	Number of categories in the dictionary.
 */
IL_EXPORT size_t il_dict_scat_cnt(il_dict_t *dict, const char *cat_id);

/**
 * Obtain the list of category IDs.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Category IDs (NULL if none or error).
 *
 * @see
 *	il_dict_scat_ids_destroy
 */
IL_EXPORT const char **il_dict_scat_ids_get(il_dict_t *dict,
					    const char *cat_id);

/**
 * Destroy the list of obtained sub-category IDs.
 *
 * @param [in] scat_ids
 *	Sub-category IDs.
 *
 * @see
 *	il_dict_scat_ids_get
 */
IL_EXPORT void il_dict_scat_ids_destroy(const char **scat_ids);

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
IL_EXPORT int il_dict_reg_get(il_dict_t *dict, const char *id,
			      const il_reg_t **reg);

/**
 * Obtain number of registers in the dictionary.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Number of registers in the dictionary.
 */
IL_EXPORT size_t il_dict_reg_cnt(il_dict_t *dict);

/**
 * Update storage value of a certain register.
 *
 * @param [in] dict
 *	Dictionary instance.
 * @param [in] id
 *	Register ID.
 * @param [in] storage
 *	Storage value.
 *
 * @return
 *	0 on success, error code otherwise.
 */
IL_EXPORT int il_dict_reg_storage_update(il_dict_t *dict, const char *id,
					 il_reg_value_t storage);
/**
 * Obtain the list of register IDs.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Dictionary IDs (NULL if none or error).
 *
 * @see
 *	il_dict_reg_ids_destroy
 */
IL_EXPORT const char **il_dict_reg_ids_get(il_dict_t *dict);

/**
 * Destroy the list of obtained register IDs.
 *
 * @param [in] regs
 *	Register list.
 *
 * @see
 *	il_dict_reg_ids_get
 */
IL_EXPORT void il_dict_reg_ids_destroy(const char **regs);


/**
 * Obtain the version of the dictionary.
 *
 * @param [in] dict
 *	Dictionary instance.
 *
 * @return
 *	Dictionary Version.
 *
 */
IL_EXPORT const char *il_dict_version_get(il_dict_t *dict);


/** @} */

IL_END_DECL

#endif

