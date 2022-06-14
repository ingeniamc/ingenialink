#include "dict_labels.h"

#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

il_dict_labels_t *il_dict_labels_create()
{
	il_dict_labels_t *labels;

	labels = malloc(sizeof(*labels));
	if (!labels) {
		ilerr__set("Labels dictionary allocation failed");
		return NULL;
	}

	/* create hash table for labels */
	labels->h = kh_init(str);
	if (!labels->h) {
		ilerr__set("Labels hash table allocation failed");
		goto cleanup_labels;
	}

	return labels;

cleanup_labels:
	free(labels);
	return NULL;
}

void il_dict_labels_destroy(il_dict_labels_t *labels)
{
	khint_t k;

	for (k = 0; k < kh_end(labels->h); ++k) {
		if (kh_exist(labels->h, k)) {
			free((char *)kh_key(labels->h, k));
			free((char *)kh_val(labels->h, k));
		}
	}

	kh_destroy(str, labels->h);

	free(labels);
}

int il_dict_labels_get(il_dict_labels_t *labels, const char *lang,
		       const char **label)
{
	khint_t k;

	k = kh_get(str, labels->h, lang);
	if (k == kh_end(labels->h)) {
		ilerr__set("Language not available (%s)", lang);
		return IL_EFAIL;
	}

	*label = kh_value(labels->h, k);

	return 0;
}

void il_dict_labels_set(il_dict_labels_t *labels, const char *lang,
			const char *label)
{
	int absent;
	khint_t k;

	k = kh_put(str, labels->h, lang, &absent);
	if (absent)
		kh_key(labels->h, k) = strdup(lang);
	else
		free((char *)kh_val(labels->h, k));

	kh_val(labels->h, k) = strdup(label);
}

void il_dict_labels_del(il_dict_labels_t *labels, const char *lang)
{
	khint_t k;

	k = kh_get(str, labels->h, lang);
	if (k != kh_end(labels->h)) {
		free((char *)kh_key(labels->h, k));
		free((char *)kh_val(labels->h, k));

		kh_del(str, labels->h, k);
	}
}

size_t il_dict_labels_nlabels_get(il_dict_labels_t *labels)
{
	return (size_t)kh_size(labels->h);
}

const char **il_dict_labels_langs_get(il_dict_labels_t *labels)
{
	const char **langs;
	size_t i;
	khint_t k;

	/* allocate array for dictister keys */
	langs = malloc(sizeof(char *) *
		       (il_dict_labels_nlabels_get(labels) + 1));
	if (!langs) {
		ilerr__set("Languages array allocation failed");
		return NULL;
	}

	/* assign keys, null-terminate */
	for (i = 0, k = 0; k < kh_end(labels->h); ++k) {
		if (kh_exist(labels->h, k)) {
			langs[i] = (const char *)kh_key(labels->h, k);
			i++;
		}
	}

	langs[i] = NULL;

	return langs;
}

void il_dict_labels_langs_destroy(const char **langs)
{
	free((char **)langs);
}

