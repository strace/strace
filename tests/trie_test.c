/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "trie.h"

#include <stdio.h>
#include <inttypes.h>

static void
assert_equals(const char *msg, uint64_t expected, uint64_t actual) {
	if (actual != expected)
		error_msg_and_fail("%s: expected: %" PRIu64
		                   ", actual: %" PRIu64, msg, expected, actual);
}

static void
iterate_fn(void *data, uint64_t key, uint64_t value)
{
	uint64_t expected = key < 256 && key % 10 == 0 ? key + 42 : -1ULL;
	assert_equals("iterate_fn", expected, value);

	int *count = (int *) data;
	if (value != -1ULL)
		(*count)++;
}

static void
test_trie_iterate_fn(void)
{
	struct trie *t = trie_create(8, 6, 3, 3, -1);
	for (int i = 0; i < 26; i++)
		trie_set(t, i * 10, i * 10 + 42);

	static const struct {
		uint64_t start;
		uint64_t end;
		int expected_count;
	} iterate_params[] = {
		{0, 256, 26},
		{0, UINT64_MAX, 26},
		{20, 90, 8},
		{99, 999, 16},
		{10000, 100000, 0},
		{200, 100, 0},
	};

	for (size_t i = 0; i < ARRAY_SIZE(iterate_params); i++) {
		int count = 0;
		trie_iterate_keys(t, iterate_params[i].start, iterate_params[i].end, iterate_fn, &count);
		assert_equals("iteration count", iterate_params[i].expected_count, count);
	}
}

struct key_value_pair {
	uint64_t key, value;
};

static void
test_trie_get(void)
{
	static const struct {
		uint8_t key_size;
		uint8_t item_size_lg;
		uint8_t node_key_bits;
		uint8_t data_block_key_bits;
		uint64_t empty_value;

		struct key_value_pair set_values[3], get_values[3];
	} params[] = {
		{64, 6, 10, 10, 0,
			{{300, 1}, {0xfacefeed, 0xabcdef123456}, {-1ULL, -1ULL}},
			{{301, 0}, {0xfacefeed, 0xabcdef123456}, {-1ULL, -1ULL}}},
		{8, 2, 4, 4, 10,
			{{0xab, 0xcd}, {0xface, 2}, {0, 3}},
			{{0xab, 0xd}, {0xface, 10}, {0, 3}}},
		{30, 0, 6, 3, -1,
			{{0xaaaa, 127}, {0xface, 0}, {0, 0}},
			{{0xaaaa, 1}, {0xface, 0}, {1, 1}}},
		{16, 4, 5, 11, 0xffffff,
			{{0xabcdef, 42}, {0xabcd, 42}, {0xffff, 0}},
			{{0xabcdef, 0xffff}, {0xabcd, 42}, {0xffff, 0}}},
		{41, 5, 1, 1, -1,
			{{0xabcdef01, 0x22222222}, {-1, 0x33333333}, {10, 10}},
			{{0xabcdef01, 0x22222222}, {-1, 0xffffffff}, {10, 10}}},
	};

	for (size_t i = 0; i < ARRAY_SIZE(params); i++) {
		struct trie *t = trie_create(params[i].key_size,
		                             params[i].item_size_lg,
					     params[i].node_key_bits,
					     params[i].data_block_key_bits,
					     params[i].empty_value);

		if (!t)
			error_msg_and_fail("trie creation failed");

		for (size_t j = 0; j < ARRAY_SIZE(params[i].set_values); j++) {
			struct key_value_pair kv = params[i].set_values[j];
			trie_set(t, kv.key, kv.value);
		}
		for (size_t j = 0; j < ARRAY_SIZE(params[i].get_values); j++) {
			struct key_value_pair kv = params[i].get_values[j];
			assert_equals("trie_get", kv.value, trie_get(t, kv.key));
		}

		trie_free(t);
	}
}

int
main(void)
{
	test_trie_get();
	test_trie_iterate_fn();
	return 0;
}
