/*
 * Helper header containing common code for finit_module, init_module,
 * and delete_module tests.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_INIT_DELETE_MODULE_H
# define STRACE_TESTS_INIT_DELETE_MODULE_H

# include <stdbool.h>
# include <stdio.h>

enum {
	PARAM1_LEN = 33,
	PARAM2_LEN = 8,
	PARAM1_BASE = 0x30,
	PARAM2_BASE = 0x80,
	MAX_STRLEN = 32,
};

static void
print_str(unsigned int base, unsigned int len, bool escape)
{
	if (!escape) {
		for (unsigned int i = base; i < (base + len); ++i)
			putc(i, stdout);
	} else {
		for (unsigned int i = base; i < (base + len); ++i)
			printf("\\%u%u%u",
			       (i >> 6) & 0x3, (i >> 3) & 0x7, i & 0x7);
	}
}

#endif /* !STRACE_TESTS_INIT_DELETE_MODULE_H */
