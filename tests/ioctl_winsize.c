/*
 * Check decoding of ioctl TIOC[GS]WINSZ commands.
 *
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/ioctl.h>

int
main(void)
{
	int rc;

	rc = ioctl(-1, TIOCGWINSZ, 0);
	printf("ioctl(-1, TIOCGWINSZ, NULL) = %s\n", sprintrc(rc));

	rc = ioctl(-1, TIOCSWINSZ, 0);
	printf("ioctl(-1, TIOCSWINSZ, NULL) = %s\n", sprintrc(rc));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct winsize, ws);
	const void *const efault = ws + 1;
	fill_memory(ws, sizeof(*ws));

	rc = ioctl(-1, TIOCSWINSZ, efault);
	printf("ioctl(-1, TIOCSWINSZ, %p) = %s\n", efault, sprintrc(rc));

	rc = ioctl(-1, TIOCGWINSZ, ws);
	printf("ioctl(-1, TIOCGWINSZ, %p) = %s\n", ws, sprintrc(rc));

	rc = ioctl(-1, TIOCSWINSZ, ws);
	printf("ioctl(-1, TIOCSWINSZ, {ws_row=%u, ws_col=%u"
	       ", ws_xpixel=%u, ws_ypixel=%u}) = %s\n",
	       ws->ws_row, ws->ws_col, ws->ws_xpixel, ws->ws_ypixel,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
