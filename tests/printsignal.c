/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "tests.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const pid_t pid = getpid();
	int rc = kill(pid, SIGCONT);
#if XLAT_RAW
	printf("kill(%d, %d) = %s\n", pid, SIGCONT, sprintrc(rc));
#elif XLAT_VERBOSE
	printf("kill(%d, %d /* SIGCONT */) = %s\n", pid, SIGCONT, sprintrc(rc));
#else /* XLAT_ABBREV */
	printf("kill(%d, SIGCONT) = %s\n", pid, sprintrc(rc));
#endif

	static const int sigs[] = { 0, 256, -1 };
	for (unsigned int i = 0; i < ARRAY_SIZE(sigs); ++i) {
		rc = kill(pid, sigs[i]);
		printf("kill(%d, %d) = %s\n", pid, sigs[i], sprintrc(rc));
	}

	printf("+++ exited with 0 +++\n");
	return 0;
}
