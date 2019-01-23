/*
 * MIT License
 *
 * Copyright (c) 2018 Ingenia-CAT S.L.
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

#ifndef PUBLIC_INGENIALINK_DICT_LABELS_H_
#define PUBLIC_INGENIALINK_DICT_LABELS_H_

#include "common.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/dict_labels.h
 * @brief Labels dictionary.
 * @defgroup IL_DICT_LABELS Labels dictionary
 * @ingroup IL
 * @{
 */

/** Labels dictionary. */
typedef struct il_dict_labels il_dict_labels_t;

/**
 * Create a labels dictionary.
 *
 * @return
 *	Labels dictionary instance (NULL if it could not be created).
 */
IL_EXPORT il_dict_labels_t *il_dict_labels_create(void);

/**
 * Destroy a labels dictionary.
 *
 * @param [in] labels
 *	Labels dictionary.
 */
IL_EXPORT void il_dict_labels_destroy(il_dict_labels_t *labels);

/**
 * Obtain the label given a language.
 *
 * @param [in] labels
 *	Labels dictionary.
 * @param [in] lang
 *	Language (ISO code).
 * @param [out] label
 *	Label.
 *
 * @return
 *	0 if label exists for the given language, IL_EFAIL otherwise.
 */
IL_EXPORT int il_dict_labels_get(il_dict_labels_t *labels, const char *lang,
				 const char **label);

/**
 * Set the label for a given language.
 *
 * @note
 *	A copy of label is stored internally.
 *
 * @param [in] labels
 *	Labels dictionary.
 * @param [in] lang
 *	Language (ISO code).
 * @param [in] label
 *	Label.
 */
IL_EXPORT void il_dict_labels_set(il_dict_labels_t *labels, const char *lang,
				  const char *label);

/**
 * Remove the label of the given language.
 *
 * @param [in] labels
 *	Labels dictionary.
 * @param [in] lang
 *	Language (ISO code).
 */
IL_EXPORT void il_dict_labels_del(il_dict_labels_t *labels, const char *lang);

/**
 * Obtain the number of labels.
 *
 * @param [in] labels
 *	Labels dictionary.
 */
IL_EXPORT size_t il_dict_labels_nlabels_get(il_dict_labels_t *labels);

/**
 * Obtain the languages available in the labels dictionary.
 *
 * @param [in] labels
 *	Labels dictionary.
 *
 * @return
 *	Null-terminated list of languages.
 *
 * @see
 *	il_dict_labels_langs_destroy
 */
IL_EXPORT const char **il_dict_labels_langs_get(il_dict_labels_t *labels);

/**
 * Destroy a list of languages.
 *
 * @param [in] langs
 *	List of languages.
 */
IL_EXPORT void il_dict_labels_langs_destroy(const char **langs);

/** @} */

IL_END_DECL

#endif
