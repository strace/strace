/*
 * Copyright (c) 2026 Vadym Hvas.
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

/* Per-syscall stats structure */
struct call_counts {
	/* time may be total latency or system time */
	struct timespec time;
	struct timespec time_min;
	struct timespec time_max;
	struct timespec time_avg;
	uint64_t calls, errors;
};

/* Per-unknown syscall stats structure */
struct unknown_call_counts {
	kernel_ulong_t scno;
	char sys_name[sizeof("syscall_0x") + sizeof(kernel_ulong_t) * 2];
	struct call_counts call_counts;
};

static const struct timespec zero_ts;
static const struct timespec max_ts = {
	(time_t) (long long) (zero_extend_signed_to_ull((time_t) -1ULL) >> 1),
	999999999 };

struct unknown_call_counts *get_unknown_by_scno(kernel_ulong_t scno);
struct unknown_call_counts *get_unknown_by_idx(size_t idx);

void count_unknown(kernel_ulong_t scno);

size_t get_unknown_bucket_size(void);