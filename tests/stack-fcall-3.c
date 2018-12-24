/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <signal.h>
#include <unistd.h>

#include "stack-fcall.h"

int f3(int i)
{
	static int pid;

	switch (i) {
	case 1:
		return kill(pid, SIGURG);

	default:
		return (pid = getpid()) + i;
	}

}
