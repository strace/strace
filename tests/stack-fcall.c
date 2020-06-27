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

int
main(int ac, char **av)
{
#if ATTACH_MODE
	/* sleep a bit to let the tracer time to catch up */
	sleep(1);
#endif

	for (;;)
		ac += f0(ac, (unsigned long) (void *) main);
}
