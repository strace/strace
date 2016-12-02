/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "defs.h"

typedef unsigned int number_slot_t;
#define BITS_PER_SLOT (sizeof(number_slot_t) * 8)

struct number_set {
	number_slot_t *vec;
	unsigned int nslots;
	bool not;
};

struct number_set read_set;
struct number_set write_set;

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

static void
add_number_to_set(const unsigned int number, struct number_set *const set)
{
	reallocate_number_set(set, number / BITS_PER_SLOT + 1);
	number_setbit(number, set->vec);
}

bool
is_number_in_set(const unsigned int number, const struct number_set *const set)
{
	return ((number / BITS_PER_SLOT < set->nslots)
		&& number_isset(number, set->vec)) ^ set->not;
}

typedef int (*string_to_uint_func)(const char *);

/*
 * Add numbers to SET according to STR specification.
 */
static void
qualify_tokens(const char *const str, struct number_set *const set,
	       string_to_uint_func func, const char *const name)
{
	/* Clear the set. */
	if (set->nslots)
		memset(set->vec, 0, sizeof(*set->vec) * set->nslots);
	set->not = false;

	/*
	 * Each leading ! character means inversion
	 * of the remaining specification.
	 */
	const char *s = str;
handle_inversion:
	while (*s == '!') {
		set->not = !set->not;
		++s;
	}

	if (strcmp(s, "none") == 0) {
		/*
		 * No numbers are added to the set.
		 * Subsequent is_number_in_set invocations will return set->not.
		 */
		return;
	} else if (strcmp(s, "all") == 0) {
		s = "!none";
		goto handle_inversion;
	}

	/*
	 * Split the string into comma separated tokens.
	 * For each token, find out the corresponding number
	 * by calling FUNC, and add that number to the set.
	 * The absence of tokens or a negative answer
	 * from FUNC is a fatal error.
	 */
	char *copy = xstrdup(s);
	char *saveptr = NULL;
	const char *token;
	int number = -1;

	for (token = strtok_r(copy, ",", &saveptr); token;
	     token = strtok_r(NULL, ",", &saveptr)) {
		number = func(token);
		if (number < 0) {
			error_msg_and_die("invalid %s '%s'", name, token);
		}

		add_number_to_set(number, set);
	}

	free(copy);

	if (number < 0) {
		error_msg_and_die("invalid %s '%s'", name, str);
	}
}

void
qualify_read(const char *const str)
{
	qualify_tokens(str, &read_set, string_to_uint, "descriptor");
}

void
qualify_write(const char *const str)
{
	qualify_tokens(str, &write_set, string_to_uint, "descriptor");
}
