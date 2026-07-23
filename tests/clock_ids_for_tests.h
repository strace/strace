/*
 * Interface to the UAPI clock IDs used by the printclockname tests.
 *
 * Only built-in C types are used here, so this header needs no includes.
 * Keeping both libc headers and <linux/time.h> out of it prevents test
 * programs from mixing their potentially conflicting time definitions;
 * the UAPI clock IDs are obtained through the separate compilation unit.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_CLOCK_IDS_FOR_TESTS_H
# define STRACE_CLOCK_IDS_FOR_TESTS_H

struct clock_test_id {
	unsigned int val;
	const char *str;
};

const struct clock_test_id *clock_ids_for_tests(unsigned int *count);
unsigned int clock_aux_last_for_tests(void);

#endif /* STRACE_CLOCK_IDS_FOR_TESTS_H */
