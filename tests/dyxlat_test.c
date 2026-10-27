/*
 * Unit tests for dyxlat (dynamic xlat table) interface.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "dyxlat.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char *
lookup_val(const struct dyxlat *const dyxlat, const uint64_t val)
{
	const struct xlat *const x = dyxlat_get(dyxlat);

	for (size_t i = 0; i < x->size; ++i) {
		if (x->data[i].val == val)
			return x->data[i].str;
	}
	return NULL;
}

static void
assert_size(const char *const msg, const size_t expected,
	    const struct dyxlat *const dyxlat)
{
	const struct xlat *const x = dyxlat_get(dyxlat);

	if (x->size != expected)
		error_msg_and_fail("%s: expected size %zu, actual %zu",
				   msg, expected, (size_t) x->size);
}

static void
assert_str(const char *const msg, const char *const expected,
	   const char *const actual)
{
	if (!expected && !actual)
		return;
	if (!expected || !actual || strcmp(expected, actual) != 0)
		error_msg_and_fail("%s: expected \"%s\", actual \"%s\"",
				   msg, expected ? expected : "(null)",
				   actual ? actual : "(null)");
}

static void
test_empty_table(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);

	assert_size("empty table", 0, d);
	assert_str("lookup in empty table", NULL, lookup_val(d, 1));
	dyxlat_free(d);
}

static void
test_zero_capacity(void)
{
	struct dyxlat *d = dyxlat_alloc(0, XT_NORMAL);

	assert_size("zero capacity initial empty", 0, d);
	dyxlat_add_pair(d, 1, "foo", 3);
	dyxlat_add_pair(d, 2, "bar", 3);
	assert_size("zero capacity after 2 adds", 2, d);
	assert_str("zero capacity lookup 1", "foo", lookup_val(d, 1));
	assert_str("zero capacity lookup 2", "bar", lookup_val(d, 2));

	dyxlat_free(d);
}

static void
test_basic(void)
{
	struct dyxlat *d = dyxlat_alloc(4, XT_NORMAL);

	assert_size("initial empty", 0, d);

	dyxlat_add_pair(d, 1, "foo", 3);
	dyxlat_add_pair(d, 2, "bar", 3);
	assert_size("after 2 adds", 2, d);

	assert_str("lookup 1", "foo", lookup_val(d, 1));
	assert_str("lookup 2", "bar", lookup_val(d, 2));
	assert_str("lookup 3", NULL, lookup_val(d, 3));

	/* Re-adding existing entry should be a no-op */
	dyxlat_add_pair(d, 1, "foo", 3);
	assert_size("after re-add 1", 2, d);
	assert_str("lookup 1 after re-add", "foo", lookup_val(d, 1));

	dyxlat_free(d);
}

static void
test_oob_same_val_trailing_nul(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);

	/*
	 * When len includes the terminating NUL byte, xstrndup allocates
	 * only the string up to the NUL.  Re-adding the same pair with
	 * identical len must not read past the allocated buffer.
	 */
	dyxlat_add_pair(d, 42, "hello\0", 6);
	assert_size("after first add with NUL", 1, d);
	assert_str("lookup 42", "hello", lookup_val(d, 42));

	dyxlat_add_pair(d, 42, "hello\0", 6);
	assert_size("after second add with NUL", 1, d);
	assert_str("lookup 42 after second add", "hello", lookup_val(d, 42));

	dyxlat_free(d);
}

static void
test_empty_string(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);

	/* Empty string with len = 0 */
	dyxlat_add_pair(d, 10, "", 0);
	assert_size("after add empty len 0", 1, d);
	assert_str("lookup 10", "", lookup_val(d, 10));

	dyxlat_add_pair(d, 10, "", 0);
	assert_size("after re-add empty len 0", 1, d);

	/* Update existing entry with len = 1 (explicit NUL) */
	dyxlat_add_pair(d, 10, "", 1);
	assert_size("after re-add empty len 1", 1, d);
	assert_str("lookup 10", "", lookup_val(d, 10));

	dyxlat_free(d);
}

static void
test_subslice_without_nul(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);
	const char buf[] = "abcdefgh";

	dyxlat_add_pair(d, 20, buf, 3);
	assert_size("after subslice", 1, d);
	assert_str("lookup 20", "abc", lookup_val(d, 20));

	dyxlat_add_pair(d, 20, buf, 3);
	assert_size("after re-add subslice", 1, d);
	assert_str("lookup 20 after re-add", "abc", lookup_val(d, 20));

	dyxlat_free(d);
}

static void
test_update(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);

	dyxlat_add_pair(d, 30, "foobar", 6);
	assert_str("initial foobar", "foobar", lookup_val(d, 30));

	/* Update with shorter string */
	dyxlat_add_pair(d, 30, "foo", 3);
	assert_size("size after shorter update", 1, d);
	assert_str("shorter update", "foo", lookup_val(d, 30));

	/* Update with longer string */
	dyxlat_add_pair(d, 30, "foobaz", 6);
	assert_size("size after longer update", 1, d);
	assert_str("longer update", "foobaz", lookup_val(d, 30));

	/* Update with a different string of the same length */
	dyxlat_add_pair(d, 30, "foobar", 6);
	assert_size("size after same-length update", 1, d);
	assert_str("same-length update", "foobar", lookup_val(d, 30));

	/* Update a nonempty entry with len = 0 */
	dyxlat_add_pair(d, 30, "", 0);
	assert_size("size after empty update", 1, d);
	assert_str("empty update", "", lookup_val(d, 30));

	dyxlat_free(d);
}

static void
test_nul_followed_by_bytes(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);
	static const char buf[] = { 'f', 'o', 'o', '\0', 'e', 'x', 't' };

	dyxlat_add_pair(d, 50, buf, sizeof(buf));
	assert_size("after add embedded NUL", 1, d);
	assert_str("lookup 50", "foo", lookup_val(d, 50));

	/* Re-add with same buffer */
	dyxlat_add_pair(d, 50, buf, sizeof(buf));
	assert_size("after re-add embedded NUL", 1, d);

	/* Re-add with canonical string "foo" */
	dyxlat_add_pair(d, 50, "foo", 3);
	assert_size("after re-add canonical foo", 1, d);
	assert_str("lookup 50 after canonical", "foo", lookup_val(d, 50));

	dyxlat_free(d);
}

static void
test_grow_array(void)
{
	struct dyxlat *d = dyxlat_alloc(2, XT_NORMAL);
	char buf[32];

	for (uint64_t i = 1; i <= 64; ++i) {
		snprintf(buf, sizeof(buf), "val_%" PRIu64, i);
		dyxlat_add_pair(d, i, buf, strlen(buf));
	}

	assert_size("after growing to 64 items", 64, d);

	for (uint64_t i = 1; i <= 64; ++i) {
		snprintf(buf, sizeof(buf), "val_%" PRIu64, i);
		const char *const actual = lookup_val(d, i);

		if (!actual || strcmp(buf, actual) != 0)
			error_msg_and_fail("lookup grown item %" PRIu64
					   ": expected \"%s\", actual \"%s\"",
					   i, buf, actual ? actual : "(null)");
	}

	dyxlat_free(d);
}

int
main(void)
{
	test_empty_table();
	test_zero_capacity();
	test_basic();
	test_oob_same_val_trailing_nul();
	test_empty_string();
	test_subslice_without_nul();
	test_update();
	test_nul_followed_by_bytes();
	test_grow_array();
	return 0;
}
