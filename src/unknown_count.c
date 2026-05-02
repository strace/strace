/*
 * Copyright (c) 2026 Vadym Hvas.
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "count.h"
#include "xstring.h"

struct unknown_call_bucket {
	size_t len;
	size_t cap;
	struct unknown_call_counts *entries;
};

#define UNKNOWN_COUNTS_ENTRIES 16

static struct unknown_call_bucket *unknown_counts;

static void
unknown_init(void)
{
	unknown_counts = xmalloc(sizeof(*unknown_counts));

	unknown_counts->len = 0;
	unknown_counts->cap = UNKNOWN_COUNTS_ENTRIES;
	unknown_counts->entries = xcalloc(UNKNOWN_COUNTS_ENTRIES,
					sizeof(*unknown_counts->entries));

	for (size_t i = 0; i < UNKNOWN_COUNTS_ENTRIES; i++)
		unknown_counts->entries->call_counts.time_min = max_ts;
}

static void
unknown_insert(kernel_ulong_t scno)
{
	if (unknown_counts->len == unknown_counts->cap) {
		unknown_counts->cap *= 2;
		unknown_counts->entries = xreallocarray(unknown_counts->entries, 
							sizeof(*unknown_counts->entries),
							unknown_counts->cap);

		for (size_t i = unknown_counts->len; i < unknown_counts->cap; i++)
			unknown_counts->entries[i].call_counts.time_min = max_ts;
	}

	unknown_counts->entries[unknown_counts->len].scno = scno;
	xsprintf(unknown_counts->entries[unknown_counts->len].sys_name,
					"syscall_%#" PRI_klx, scno);
	unknown_counts->len++;
}

struct unknown_call_counts *
get_unknown_by_scno(kernel_ulong_t scno)
{
	for (size_t i = 0; i < unknown_counts->len; i++)
		if (unknown_counts->entries[i].scno == scno)
			return &unknown_counts->entries[i];

	return NULL;
}

struct unknown_call_counts *
get_unknown_by_idx(size_t idx)
{
        if (!unknown_counts)
                return NULL;

        if (idx >= unknown_counts->len)
                return NULL;

        return &unknown_counts->entries[idx];
}

void
count_unknown(kernel_ulong_t scno)
{
        if (!unknown_counts)
                unknown_init();

        if (!get_unknown_by_scno(scno))
                unknown_insert(scno);
}

size_t
get_unknown_bucket_size(void)
{
	return unknown_counts ? unknown_counts->len : 0;
}