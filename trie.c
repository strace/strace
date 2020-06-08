/*
 * Simple trie implementation for key-value mapping storage
 *
 * Copyright (c) 2020 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "trie.h"
#include "xmalloc.h"

static const uint8_t ptr_sz_lg = (sizeof(void *) == 8 ? 6 : 5);

/**
 * Returns lg2 of node size in bits for the specific level of the trie.
 */
static uint8_t
trie_get_node_size(struct trie *t, uint8_t depth)
{
	/* Last level contains data and we allow it having a different size */
	if (depth == t->max_depth)
		return t->data_block_key_bits + t->item_size_lg;
	/* Last level of the tree can be smaller */
	if (depth == t->max_depth - 1)
		return (t->key_size - t->data_block_key_bits - 1) %
		t->node_key_bits + 1 + ptr_sz_lg;

	return t->node_key_bits + ptr_sz_lg;
}

/**
 * Provides starting offset of bits in key corresponding to the node index
 * at the specific level.
 */
static uint8_t
trie_get_node_bit_offs(struct trie *t, uint8_t depth)
{
	uint8_t offs;

	if (depth == t->max_depth)
		return 0;

	offs = t->data_block_key_bits;

	if (depth == t->max_depth - 1)
		return offs;

	/* data_block_size + remainder */
	offs += trie_get_node_size(t, t->max_depth - 1) - ptr_sz_lg;
	offs += (t->max_depth - depth - 2) * t->node_key_bits;

	return offs;
}

struct trie *
trie_create(uint8_t key_size, uint8_t item_size_lg, uint8_t node_key_bits,
            uint8_t data_block_key_bits, uint64_t empty_value)
{
	if (item_size_lg > 6)
		return NULL;
	if (key_size > 64)
		return NULL;
	if (node_key_bits < 1)
		return NULL;
	if (data_block_key_bits < 1 || data_block_key_bits > key_size)
		return NULL;

	struct trie *t = malloc(sizeof(*t));
	if (!t)
		return NULL;

	t->empty_value = empty_value;
	t->data = NULL;
	t->item_size_lg = item_size_lg;
	t->node_key_bits = node_key_bits;
	t->data_block_key_bits = data_block_key_bits;
	t->key_size = key_size;
	t->max_depth = (key_size - data_block_key_bits + node_key_bits - 1)
		/ t->node_key_bits;

	if (item_size_lg != 6)
		t->empty_value &= (((uint64_t) 1 << (1 << t->item_size_lg)) - 1);

	return t;
}

static void *
trie_create_data_block(struct trie *t)
{
	uint64_t fill_value = t->empty_value;
	for (int i = 1; i < 1 << (6 - t->item_size_lg); i++) {
		fill_value <<= (1 << t->item_size_lg);
		fill_value |= t->empty_value;
	}

	uint8_t sz = t->data_block_key_bits + t->item_size_lg;
	if (sz < 6)
		sz = 6;

	size_t count = 1 << (sz - 6);
	uint64_t *data_block = xcalloc(count, 8);

	for (size_t i = 0; i < count; i++)
		data_block[i] = fill_value;

	return data_block;
}

static uint64_t *
trie_get_node(struct trie *t, uint64_t key, bool auto_create)
{
	void **cur_node = &(t->data);

	if (t->key_size < 64 && key > (uint64_t) 1 << t->key_size)
		return NULL;

	for (uint8_t cur_depth = 0; cur_depth <= t->max_depth; cur_depth++) {
		uint8_t offs = trie_get_node_bit_offs(t, cur_depth);
		uint8_t sz = trie_get_node_size(t, cur_depth);

		if (!*cur_node) {
			if (!auto_create)
				return NULL;

			if (cur_depth == t->max_depth)
				*cur_node = trie_create_data_block(t);
			else
				*cur_node = xcalloc(1 << sz, 1);
		}

		if (cur_depth == t->max_depth)
			break;

		size_t pos = (key >> offs) & ((1 << (sz - ptr_sz_lg)) - 1);
		cur_node = (((void **) (*cur_node)) + pos);
	}

	return (uint64_t *) (*cur_node);
}

static void
trie_data_block_calc_pos(struct trie *t, uint64_t key,
                         uint64_t *pos, uint64_t *mask, uint64_t *offs)
{
	uint64_t key_mask;

	key_mask = (1 << t->data_block_key_bits) - 1;
	*pos = (key & key_mask) >> (6 - t->item_size_lg);

	if (t->item_size_lg == 6) {
		*offs = 0;
		*mask = -1;
		return;
	}

	key_mask = (1 << (6 - t->item_size_lg)) - 1;
	*offs = (key & key_mask) * (1 << t->item_size_lg);

	*mask = (((uint64_t) 1 << (1 << t->item_size_lg)) - 1) << *offs;
}

bool
trie_set(struct trie *t, uint64_t key, uint64_t val)
{
	uint64_t *data = trie_get_node(t, key, true);
	if (!data)
		return false;

	uint64_t pos, mask, offs;
	trie_data_block_calc_pos(t, key, &pos, &mask, &offs);

	data[pos] &= ~mask;
	data[pos] |= (val << offs) & mask;

	return true;
}

static uint64_t
trie_data_block_get(struct trie *t, uint64_t *data, uint64_t key)
{
	if (!data)
		return t->empty_value;

	uint64_t pos, mask, offs;
	trie_data_block_calc_pos(t, key, &pos, &mask, &offs);

	return (data[pos] & mask) >> offs;
}

uint64_t
trie_get(struct trie *b, uint64_t key)
{
	return trie_data_block_get(b, trie_get_node(b, key, false), key);
}

static uint64_t
trie_iterate_keys_node(struct trie *t,
                       trie_iterate_fn fn, void *fn_data,
                       void *node, uint64_t start, uint64_t end,
                       uint8_t depth)
{
	if (start > end || !node)
		return 0;

	if (t->key_size < 64) {
		uint64_t key_max = ((uint64_t) 1 << t->key_size) - 1;
		if (end > key_max)
			end = key_max;
	}

	if (depth == t->max_depth) {
		for (uint64_t i = start; i <= end; i++)
			fn(fn_data, i, trie_data_block_get(t,
				(uint64_t *) node, i));

		return end - start + 1;
	}

	uint8_t parent_node_bit_off = depth == 0 ?
		t->key_size :
		trie_get_node_bit_offs(t, depth - 1);

	uint64_t first_key_in_node = start &
		(uint64_t) -1 << parent_node_bit_off;

	uint8_t node_bit_off = trie_get_node_bit_offs(t, depth);
	uint8_t node_key_bits = parent_node_bit_off - node_bit_off;
	uint64_t mask = ((uint64_t) 1 << (node_key_bits)) - 1;
	uint64_t start_index = (start >> node_bit_off) & mask;
	uint64_t end_index = (end >> node_bit_off) & mask;
	uint64_t child_key_count = (uint64_t) 1 << node_bit_off;

	uint64_t count = 0;

	for (uint64_t i = start_index; i <= end_index; i++) {
		uint64_t child_start = first_key_in_node + i * child_key_count;
		uint64_t child_end = first_key_in_node +
			(i + 1) * child_key_count - 1;

		if (child_start < start)
			child_start = start;
		if (child_end > end)
			child_end = end;

		count += trie_iterate_keys_node(t, fn, fn_data,
			((void **) node)[i], child_start, child_end,
			depth + 1);
	}

	return count;
}

uint64_t trie_iterate_keys(struct trie *t, uint64_t start, uint64_t end,
                           trie_iterate_fn fn, void *fn_data)
{
	return trie_iterate_keys_node(t, fn, fn_data, t->data,
		start, end, 0);
}

static void
trie_free_node(struct trie *t, void *node, uint8_t depth)
{
	if (!node)
		return;

	if (depth >= t->max_depth)
		goto free_node;

	size_t sz = 1 << (trie_get_node_size(t, depth) - ptr_sz_lg);
	for (size_t i = 0; i < sz; i++)
		trie_free_node(t, ((void **) node)[i], depth + 1);

free_node:
	free(node);
}

void
trie_free(struct trie *t)
{
	trie_free_node(t, t->data, 0);
	free(t);
}
