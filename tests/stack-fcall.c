/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "stack-fcall.h"

#ifndef ATTACH_MODE
# define ATTACH_MODE 0
#endif

int main(void)
{
#if ATTACH_MODE
	/* sleep a bit to let the tracer time to catch up */
	sleep(1);
#endif

	f0(0, (unsigned long) (void *) main);
	f0(1, (unsigned long) (void *) main);
	return 0;
}
