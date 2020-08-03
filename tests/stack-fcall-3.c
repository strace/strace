/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <signal.h>
#include "stack-fcall.h"

int
f3(int i, unsigned long f)
{
	f ^= (unsigned long) (void *) f3;
	COMPLEX_BODY(i, f);
	switch (i) {
	case 1:
		i -= chdir("");
		break;
	case 2:
		i -= kill(getpid(), SIGURG) - 1;
		break;
	default:
		i -= syscall(__NR_exit, i - 3);
		break;
	}
	return i;
}
