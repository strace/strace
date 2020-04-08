/*
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <signal.h>
#include <unistd.h>
#include "scno.h"

#include "stack-fcall.h"

int f3(int i, unsigned long f)
{
	syscall(__NR_gettid, f ^ (unsigned long) (void *) f3);
	switch (i) {
	case 1:
		return kill(getpid(), SIGURG);

	default:
		return chdir("") + i;
	}

}
