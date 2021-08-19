/*
 * Simple trie interface
 *
 * Copyright (c) 2020-2021 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TRIE_H
# define STRACE_TRIE_H

# include <stdbool.h>
# include <stdint.h>

/**
 * Trie control structure.
 * Trie implemented here has the following properties:
 *  * It allows storing values of the same size, the size can vary from 1 bit to
 *    64 bit values (only power of 2 sizes are allowed).
 *  * The key can be up to 64 bits in size.
 *  * It has separate configuration for node size and data block size.
 *
 * How bits of key are used for different node levels:
 *
 *   highest bits                                                  lowest bits
 *  | node_key_bits | node_key_bits | ... | <remainder> | data_block_key_bits |
 *  \_________________________________________________________________________/
 *                                 key_size
 *
 * So, the remainder is used on the lowest non-data node level.
 *
 * As of now, it doesn't implement any mechanisms for resizing/changing key
 * size.  De-fragmentation is also unsupported currently.
 */
struct trie {
	/** Return value of trie_get if key is not found */
	uint64_t empty_value;
	/**
	 * Empty value copied over to fill the whole uint64_t.
	 * Pre-calculated in trie_create to be used in trie_create_data_block.
	 */
	uint64_t fill_value;

	/** Pointer to root node */
	void *data;

	/** Key size in bits (0..64). */
	uint8_t key_size;

	/**
	 * Size of the stored values in log2 bits (0..6).
	 * (6: 64 bit values, 5: 32 bit values, ...)
	 */
	uint8_t item_size_lg;

	/**
	 * Number of bits in the key that make a symbol for a node.
	 * (equals to log2 of the child count of the node)
	 */
	uint8_t node_key_bits;

	/**
	 * Number of bits in the key that make a symbol for the data block (leaf).
	 * (equals to log2 of the value count stored in a data block)
	 */
	uint8_t data_block_key_bits;

	/** The depth of the data block. Calculated from the values above */
	uint8_t max_depth;
};

struct trie* trie_create(uint8_t key_size, uint8_t item_size_lg,
			uint8_t node_key_bits, uint8_t data_block_key_bits,
			uint64_t empty_value);

bool trie_set(struct trie *t, uint64_t key, uint64_t val);
uint64_t trie_get(struct trie *t, uint64_t key);

typedef void (*trie_iterate_fn)(void *data, uint64_t key, uint64_t val);

/**
 * Calls trie_iterate_fn for each key-value pair where
 * key is inside the [start, end] interval (inclusive).
 *
 * @param t        The trie.
 * @param start    The start of the key interval (inclusive).
 * @param end      The end of the key interval (inclusive).
 * @param fn       The function to be called.
 * @param fn_data  The value to be passed to fn.
 */
uint64_t trie_iterate_keys(struct trie *t, uint64_t start, uint64_t end,
			    trie_iterate_fn fn, void *fn_data);

void trie_free(struct trie *t);

#endif /* !STRACE_TRIE_H */
