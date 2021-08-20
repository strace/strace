/*
 * Copyright (c) 2017-2021 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

struct dyxlat {
	size_t allocated;
	struct xlat xlat;
	struct xlat_data *data;
};

struct dyxlat *
dyxlat_alloc(const size_t nmemb, enum xlat_type type)
{
	struct dyxlat *const dyxlat = xmalloc(sizeof(*dyxlat));

	dyxlat->xlat.type = type;
	dyxlat->xlat.size = 0;
	dyxlat->allocated = nmemb;
	dyxlat->xlat.data = dyxlat->data = xgrowarray(NULL, &dyxlat->allocated,
						      sizeof(struct xlat_data));

	return dyxlat;
}

void
dyxlat_free(struct dyxlat *const dyxlat)
{
	for (size_t i = 0; i < dyxlat->xlat.size; ++i) {
		free((void *) dyxlat->data[i].str);
		dyxlat->data[i].str = NULL;
	}

	free(dyxlat->data);
	dyxlat->xlat.data = NULL;
	free(dyxlat);
}

const struct xlat *
dyxlat_get(const struct dyxlat *const dyxlat)
{
	return &dyxlat->xlat;
}

void
dyxlat_add_pair(struct dyxlat *const dyxlat, const uint64_t val,
		const char *const str, const size_t len)
{
	for (size_t i = 0; i < dyxlat->xlat.size; ++i) {
		if (dyxlat->data[i].val == val) {
			if (strncmp(dyxlat->data[i].str, str, len) == 0
			    && dyxlat->data[i].str[len] == '\0')
				return;

			free((void *) dyxlat->data[i].str);
			dyxlat->data[i].str = xstrndup(str, len);
			return;
		}
	}

	if (dyxlat->xlat.size >= dyxlat->allocated)
		dyxlat->xlat.data = dyxlat->data =
			xgrowarray(dyxlat->data, &dyxlat->allocated,
				   sizeof(struct xlat_data));

	dyxlat->data[dyxlat->xlat.size].val = val;
	dyxlat->data[dyxlat->xlat.size].str = xstrndup(str, len);
	dyxlat->xlat.size++;
}
