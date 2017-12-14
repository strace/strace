/*
 * Copyright (c) 2017 The strace developers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
