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

struct unknown_call_counts *get_unknown_by_scno(kernel_ulong_t scno);
struct unknown_call_counts *get_unknown_by_idx(size_t idx);

void count_unknown(kernel_ulong_t scno, struct timespec time_max);