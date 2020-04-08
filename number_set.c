/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "number_set.h"
#include "static_assert.h"
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

static unsigned int
get_number_setbit(const struct number_set *const set)
{
	static_assert(sizeof(number_slot_t) == sizeof(uint32_t),
		      "number_slot_t is not 32-bit long");
	return popcount32(set->vec, set->nslots);
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

bool
is_complete_set(const struct number_set *const set, const unsigned int max_numbers)
{
	return set && ((set->not && !set->nslots) ||
		       (get_number_setbit(set) == max_numbers));
}

bool
is_complete_set_array(const struct number_set *const set,
		      const unsigned int *const max_numbers,
		      const unsigned int nmemb)
{
	for (unsigned int i = 0; i < nmemb; ++i) {
		if (!is_complete_set(&set[i], max_numbers[i]))
			return false;
	}
	return true;
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
