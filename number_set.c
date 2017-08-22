/*
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "number_set.h"
#include "xmalloc.h"

typedef unsigned int number_slot_t;
#define BITS_PER_SLOT (sizeof(number_slot_t) * 8)

struct number_set {
	number_slot_t *vec;
	unsigned int nslots;
	bool not;
};

static void
number_setbit(const unsigned int i, number_slot_t *const vec)
{
	vec[i / BITS_PER_SLOT] |= (number_slot_t) 1 << (i % BITS_PER_SLOT);
}

static bool
number_isset(const unsigned int i, const number_slot_t *const vec)
{
	return vec[i / BITS_PER_SLOT] & ((number_slot_t) 1 << (i % BITS_PER_SLOT));
}

static void
reallocate_number_set(struct number_set *const set, const unsigned int new_nslots)
{
	if (new_nslots <= set->nslots)
		return;
	set->vec = xreallocarray(set->vec, new_nslots, sizeof(*set->vec));
	memset(set->vec + set->nslots, 0,
	       sizeof(*set->vec) * (new_nslots - set->nslots));
	set->nslots = new_nslots;
}

bool
number_set_array_is_empty(const struct number_set *const set,
			  const unsigned int idx)
{
	return !(set && (set[idx].nslots || set[idx].not));
}

bool
is_number_in_set(const unsigned int number, const struct number_set *const set)
{
	return set && ((number / BITS_PER_SLOT < set->nslots)
		&& number_isset(number, set->vec)) ^ set->not;
}

bool
is_number_in_set_array(const unsigned int number, const struct number_set *const set,
		       const unsigned int idx)
{
	return set && ((number / BITS_PER_SLOT < set[idx].nslots)
		&& number_isset(number, set[idx].vec)) ^ set[idx].not;
}

void
add_number_to_set(const unsigned int number, struct number_set *const set)
{
	reallocate_number_set(set, number / BITS_PER_SLOT + 1);
	number_setbit(number, set->vec);
}

void
add_number_to_set_array(const unsigned int number, struct number_set *const set,
			const unsigned int idx)
{
	add_number_to_set(number, &set[idx]);
}

void
clear_number_set_array(struct number_set *const set, const unsigned int nmemb)
{
	unsigned int i;

	for (i = 0; i < nmemb; ++i) {
		if (set[i].nslots)
			memset(set[i].vec, 0,
			       sizeof(*set[i].vec) * set[i].nslots);
		set[i].not = false;
	}
}

void
invert_number_set_array(struct number_set *const set, const unsigned int nmemb)
{
	unsigned int i;

	for (i = 0; i < nmemb; ++i)
		set[i].not = !set[i].not;
}

struct number_set *
alloc_number_set_array(const unsigned int nmemb)
{
	return xcalloc(nmemb, sizeof(struct number_set));
}

void
free_number_set_array(struct number_set *const set, unsigned int nmemb)
{
	while (nmemb) {
		--nmemb;
		free(set[nmemb].vec);
		set[nmemb].vec = NULL;
	}
	free(set);
}
