/*
 * Check seccomp filter performance.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static volatile bool stop = false;

static void
handler(int signo)
{
	stop = true;
}

int
main(void)
{
	unsigned int i;
	int rc = 0;

	signal(SIGALRM, handler);
	alarm(1);

	for (i = 0; !stop; i++) {
		rc |= chdir(".");
	}
	printf("%d\n", i);
	return rc;
}
