#ifndef DICT_LABELS_H_
#define DICT_LABELS_H_

#include "public/ingenialink/dict_labels.h"

#include "klib/khash.h"

/** khash type for str<->label */
KHASH_MAP_INIT_STR(str, const char *)

/** Labels dictionary. */
struct il_dict_labels {
	/** Hash table. */
	khash_t(str) * h;
};

#endif

