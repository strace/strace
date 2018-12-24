/*
 * Copyright (c) 2017-2018 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

struct dyxlat {
	size_t used;
	size_t allocated;
	struct xlat *xlat;
};

#define MARK_END(xlat)				\
	do {					\
		(xlat).val = 0;			\
		(xlat).str = 0;			\
	} while (0)

struct dyxlat *
dyxlat_alloc(const size_t nmemb)
{
	struct dyxlat *const dyxlat = xmalloc(sizeof(*dyxlat));

	dyxlat->used = 1;
	dyxlat->allocated = nmemb;
	dyxlat->xlat = xgrowarray(NULL, &dyxlat->allocated, sizeof(struct xlat));
	MARK_END(dyxlat->xlat[0]);

	return dyxlat;
}

void
dyxlat_free(struct dyxlat *const dyxlat)
{
	size_t i;

	for (i = 0; i < dyxlat->used - 1; ++i) {
		free((void *) dyxlat->xlat[i].str);
		dyxlat->xlat[i].str = NULL;
	}

	free(dyxlat->xlat);
	dyxlat->xlat = NULL;
	free(dyxlat);
}

const struct xlat *
dyxlat_get(const struct dyxlat *const dyxlat)
{
	return dyxlat->xlat;
}

void
dyxlat_add_pair(struct dyxlat *const dyxlat, const uint64_t val,
		const char *const str, const size_t len)
{
	size_t i;

	for (i = 0; i < dyxlat->used - 1; ++i) {
		if (dyxlat->xlat[i].val == val) {
			if (strncmp(dyxlat->xlat[i].str, str, len) == 0
			    && dyxlat->xlat[i].str[len] == '\0')
				return;

			free((void *) dyxlat->xlat[i].str);
			dyxlat->xlat[i].str = xstrndup(str, len);
			return;
		}
	}

	if (dyxlat->used >= dyxlat->allocated)
		dyxlat->xlat = xgrowarray(dyxlat->xlat, &dyxlat->allocated,
					  sizeof(struct xlat));

	dyxlat->xlat[dyxlat->used - 1].val = val;
	dyxlat->xlat[dyxlat->used - 1].str = xstrndup(str, len);
	MARK_END(dyxlat->xlat[dyxlat->used]);
	dyxlat->used++;
}
