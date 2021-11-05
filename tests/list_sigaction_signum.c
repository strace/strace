/*
 * List signal numbers that are valid arguments for sigaction syscall.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <signal.h>
#include <stdio.h>

int
main(void)
{
	for (unsigned int i = 1; i < 32; ++i) {
		static const struct sigaction ign = { .sa_handler = SIG_IGN };
		static const struct sigaction dfl = { .sa_handler = SIG_DFL };
		struct sigaction act;

		if (sigaction(i, &ign, NULL) ||
		    sigaction(i, NULL, &act) ||
		    ign.sa_handler != act.sa_handler ||
		    sigaction(i, &dfl, NULL) ||
		    sigaction(i, NULL, &act) ||
		    dfl.sa_handler != act.sa_handler)
			continue;

		printf("%u\n", i);
	}

	return 0;
}
